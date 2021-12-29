#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <poll.h>
#include "Infra/Time.h"
#include "HeartbeatMonitor.h"
#include "Common/Defs.h"
#include "Common/os.h"
#include "MessageType.h"
#include "HALog.h"
#include "HAConfigs.h"
#include "CommonFun.h"

extern Dahua::VideoCloud::DBHA::MasterBinlogInfo gHeartBeatBinlogInfo;

#define ELECTION_LOG( old_state, new_state, judgetype, judgement ) \
	electLogInfo( "%s -> %s, judge type: %s, judgement:\n%s", \
	strState( old_state ), strState( new_state ), #judgetype, judgement.str().c_str() );

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

using namespace EFS;

DBHALOG_CLASSNAME( CHeartbeatMonitor );

//单台机器测试，需要一些特殊代码
//#define GTEST_SINGLE_COMPUTER

const int32_t maxPollTimeout = 500;       // poll最大超时时间为500ms
const int32_t lastStateTimeout = 60 * 1000; // 之前处于slave状态时，切换为master时间
const int32_t maxConnectCnt=5;			  // 当主备心跳业务同时断开5个检测后，自动备机成为主机

#define ONLY_LOCAL_DB_HAS_NEW_DATA(peerHasNewData, localHasNewData) ((!peerHasNewData) & (localHasNewData))
#define LOCAL_DB_HAS_NEWEST_DATA(peerHasNewData, localHasNewData, localIsNewMaster) \
		( localIsNewMaster ) & ( !( (peerHasNewData) ^ (localHasNewData) ))

class CHeartbeatWorker: public Infra::CThread
{
public:
    CHeartbeatWorker();
    ~CHeartbeatWorker();

    typedef Infra::TFunction1<void, int32_t> TimeoutProc;
    typedef Infra::TFunction1<int, int> ReadProc;

    int32_t setTimer( uint32_t timeout, TimeoutProc proc );
    bool resetTimer( int32_t id );
    bool destroyTimer( int32_t id );
    bool registerReadEvent( int fd, ReadProc proc ); //寄存器事件读取
    bool unregisterReadEvent( int fd );
private:
    void threadProc();
    bool checkThreadShoudStop();
private:
    typedef struct {
        uint32_t timeout;		// 超时时间
        uint32_t standTime;		// 距离下次超时剩余时间
        uint64_t resetTimeStamp;// 重置的时间戳，用于记录超时时间不正确的定时器
        TimeoutProc proc;		// 超时回调
    } TimerNode;
    std::map<int32_t, TimerNode> m_timer_map;
    int32_t m_alloc_timer_id;   // 用于生成定时器ID,以便多次调用setTimer可以返回不同的ID。
    std::map<int, ReadProc> m_fd_map;
    uint32_t m_poll_fds_size;
    struct pollfd* m_poll_fds;
    Infra::CMutex m_mutex;
    bool m_exit_flag;
};

CHeartbeatWorker::CHeartbeatWorker() :
    CThread( "Heartbeat worker", priorTop/*, policyRealtime */ )
{
    m_alloc_timer_id = 1;
    m_poll_fds_size = 0;
    m_poll_fds = NULL;
    m_exit_flag = false;
    createThread();
}

CHeartbeatWorker::~CHeartbeatWorker()
{
	DBHALOG_INFO("begin destructor...");
	m_exit_flag = true;
    destroyThread();
    if( m_poll_fds != NULL ) {
        delete[] m_poll_fds, m_poll_fds = NULL;
    }
    DBHALOG_INFO("end destructor...");
}

int32_t CHeartbeatWorker::setTimer( uint32_t timeout, TimeoutProc proc )
{
    if( timeout == 0 || proc.empty() ) {
        return -1;
    }
    int32_t ret = 0;
    m_mutex.enter();
    ret = ++m_alloc_timer_id;
    TimerNode node;
    node.timeout = timeout;
    node.standTime = timeout;
    node.resetTimeStamp = Infra::CTime::getCurrentMilliSecond();
    node.proc = proc;
    m_timer_map[ret] = node;
    m_mutex.leave();
    return ret;
}

bool CHeartbeatWorker::resetTimer( int32_t id )
{
    bool ret = false;
    m_mutex.enter();
    std::map<int32_t, TimerNode>::iterator it = m_timer_map.find( id );
    if( it != m_timer_map.end() ) {
        it->second.standTime = it->second.timeout;
        it->second.resetTimeStamp = Infra::CTime::getCurrentMilliSecond();
        ret = true;
    }
    m_mutex.leave();
    return ret;
}

bool CHeartbeatWorker::destroyTimer( int32_t id )
{
    bool ret = false;
    m_mutex.enter();
    std::map<int32_t, TimerNode>::iterator it = m_timer_map.find( id );
    if( it != m_timer_map.end() ) {
        m_timer_map.erase( it );
        ret = true;
    }
    m_mutex.leave();
    return ret;
}

bool CHeartbeatWorker::registerReadEvent( int fd, ReadProc proc )
{
    if( fd == -1 || proc.empty() ) {
        return false;
    }
    m_mutex.enter();
    std::map<int, ReadProc>::iterator it = m_fd_map.find( fd );
    ASSERT_ABORT( it == m_fd_map.end() );
    m_fd_map[fd] = proc;
    m_mutex.leave();
    return true;
}

bool CHeartbeatWorker::unregisterReadEvent( int fd )
{
    bool ret = false;
    m_mutex.enter();
    std::map<int, ReadProc>::iterator it = m_fd_map.find( fd );
    if( it != m_fd_map.end() ) {
        m_fd_map.erase( it );
        ret = true;
    }
    m_mutex.leave();
    return ret;
}

bool CHeartbeatWorker::checkThreadShoudStop()
{
	return m_exit_flag;
}

void CHeartbeatWorker::threadProc()
{
    std::map<int32_t, TimerNode> call_timer_map;
    std::map<int, ReadProc> call_read_event_map;
    int num = sysconf( _SC_NPROCESSORS_CONF );
    if( num > 0 ) {
        cpu_set_t mask;
        CPU_ZERO( &mask );
        CPU_SET( 1, &mask );
        int ret = pthread_setaffinity_np( pthread_self(), sizeof( mask ), &mask );
        if( ret != 0 ) {
            DBHALOG_ERROR( "pthread_setaffinity_np failed with error %d, cpu num %d", ret, num );
        } else {
            DBHALOG_INFO( "set affinity to:%u", 1 );
        }
    }

    while( looping() && (!checkThreadShoudStop()) ) {
        m_mutex.enter();
        if( m_fd_map.size() != m_poll_fds_size ) {
            if( m_poll_fds != NULL ) {
                delete[] m_poll_fds;
            }
            m_poll_fds_size = m_fd_map.size();
            m_poll_fds = new struct pollfd[m_poll_fds_size];
        }
        std::map<int, ReadProc>::iterator fd_it = m_fd_map.begin();
        for( uint32_t i = 0; i < m_poll_fds_size; ++i, ++fd_it ) {
            m_poll_fds[i].fd = fd_it->first;
            m_poll_fds[i].events = POLLIN;
            m_poll_fds[i].revents = 0;
        }

		// 如果最小定时器超时时间小于maxPollTimeout，就取最小超时时间作为poll的超时时间
		int32_t timeout = maxPollTimeout;
		std::map<int32_t, TimerNode>::iterator timer_it = m_timer_map.begin();
		for( ; timer_it != m_timer_map.end(); ++timer_it ){
			if( timer_it->second.standTime < (uint32_t) timeout )
				timeout = timer_it->second.standTime;
		}
		m_mutex.leave();

		int ret = 0;
		int32_t interval = 0;
		int32_t tmp = timeout;
		uint64_t poll_before = Infra::CTime::getCurrentMilliSecond();
		do{
			// 防止poll过程中被信号打断返回EINTR错误,记录本次poll的时间,如果返回EINTR,下一次poll的等待时间就是 (timeout-上次调用poll的等待时间)
			uint64_t bef_poll = Infra::CTime::getCurrentMilliSecond();
			ret = poll( m_poll_fds, m_poll_fds_size, tmp );
			interval = Infra::CTime::getCurrentMilliSecond() - bef_poll;
			tmp -= interval;
			if( tmp <= 0 || checkThreadShoudStop() ){
				break;
			}
		}while( -1 == ret && errno == EINTR );

		m_mutex.enter();
		// 记录收到数据的socket fd以及对应的回调函数
		for( uint32_t i = 0; i < m_poll_fds_size; ++i, ++fd_it ){
			if( m_poll_fds[i].revents & POLLIN ){
				call_read_event_map[m_poll_fds[i].fd] = m_fd_map[m_poll_fds[i].fd];
				m_poll_fds[i].revents = 0;
			}
		}

		uint64_t poll_cost = Infra::CTime::getCurrentMilliSecond() - poll_before;

		// 记录有哪些定时器已经超时,对于超时的定时器,记录定时ID并将其重置
		timer_it = m_timer_map.begin();
		uint32_t time_using = poll_cost;
		for( ; timer_it != m_timer_map.end(); ++timer_it ){
			if( timer_it->second.standTime <= time_using ){
				timer_it->second.standTime = timer_it->second.timeout;
				call_timer_map[timer_it->first] = timer_it->second;
				uint64_t timeStamp = Infra::CTime::getCurrentMilliSecond();
				uint32_t timeSpace = timeStamp - timer_it->second.resetTimeStamp;
				timer_it->second.resetTimeStamp = Infra::CTime::getCurrentMilliSecond();
				if( timeSpace >= timer_it->second.timeout + 500 ){
                    DBHALOG_INFO( "timer[%d] large timeout [%d,%d]", timer_it->first, timeSpace, timer_it->second.timeout );
				}
			}else
				timer_it->second.standTime -= time_using;
		}
		m_mutex.leave();

		if( checkThreadShoudStop() ){
			break;
		}

		// 处理fd收到的消息: 调用事先注册的消息回调函数
		fd_it = call_read_event_map.begin();
		for( ; fd_it != call_read_event_map.end(); ){
			fd_it->second( fd_it->first );
			call_read_event_map.erase( fd_it++ );
		}
		
		// 处理超时消息: 调用事先注册的定时器回调函数
		timer_it = call_timer_map.begin();
		for( ; timer_it != call_timer_map.end(); ){
			timer_it->second.proc( timer_it->first );
			call_timer_map.erase( timer_it++ );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// HA事件管理(异步)回调类,处理两类事件:
// 1. 本机HA状态变化; 2.远端机器上下线

class CStateCallback: public Infra::CThread
{
public:
    CStateCallback();
    ~CStateCallback();

    // 设置HA状态、远端HA上下线事件的回调函数
    void setStateProc( CHeartbeatMonitor::HAStateProc proc );

    // 设置HA消息处理回调函数(通过该回调将消息交给上层模块处理)
    void setMsgProc( CHeartbeatMonitor::MsgHandlerProc proc );

    // CHeartbeatMonitor调用该接口添加一个HA状态变化事件,然后CStateCallback类内部异步回调上层接口
    void pushState( CHeartbeatMonitor::HAAction action, CHeartbeatMonitor::LinkState& links );

	// 添加HA状态事件，并等待事件处理结束
	void pushStateAndWait( CHeartbeatMonitor::HAAction action, CHeartbeatMonitor::LinkState& links );
	
    // 添加心跳消息
    void pushMessage( CHeartbeatMonitor::HAState localState, CHeartbeatMonitor::HAState peerState,
                      TMulticastHeatBeatMsgPtr msg );
private:
    void threadProc();
private:
    Infra::CSemaphore m_sem;

    //消息回调相关变量
    typedef struct {
        CHeartbeatMonitor::HAState  localState;
        CHeartbeatMonitor::HAState  peerState;
        TMulticastHeatBeatMsgPtr  msg;
    } HbMsgArgs;
    std::list<HbMsgArgs> m_msg_list;                       // 本机收到的消息列表,用于回调通知上层
    CHeartbeatMonitor::MsgHandlerProc m_msg_proc;          // 消息回调

    typedef struct {
        CHeartbeatMonitor::HAAction action;
        CHeartbeatMonitor::LinkState link;
		Infra::CSemaphore* wait; ///<  用于等待action处理结束
    } HbStateArgs;
    std::list<HbStateArgs> m_state_list;
    CHeartbeatMonitor::HAStateProc m_ha_stat_proc;         // ha状态变化回调
    bool m_exit_flag;

};

CStateCallback::CStateCallback() :
    CThread( "Heartbeat callback" )
{
    m_exit_flag = false;
    createThread();
}

CStateCallback::~CStateCallback()
{
	DBHALOG_INFO("begin destructor...");
    m_exit_flag = true;
    m_sem.post();
    destroyThread();
    DBHALOG_INFO("end destructor...");
}

void CStateCallback::setStateProc( CHeartbeatMonitor::HAStateProc proc )
{
    m_ha_stat_proc = proc;
}

void CStateCallback::setMsgProc( CHeartbeatMonitor::MsgHandlerProc proc )
{
    m_msg_proc = proc;
}

void CStateCallback::pushState( CHeartbeatMonitor::HAAction action, CHeartbeatMonitor::LinkState& links )
{
    HbStateArgs args;
    args.action = action;
    args.link = links;
	args.wait = NULL;
    m_state_list.push_back( args );
    m_sem.post();
}

void CStateCallback::pushStateAndWait( CHeartbeatMonitor::HAAction action, CHeartbeatMonitor::LinkState& links )
{
	HbStateArgs args;
    args.action = action;
    args.link = links;
	args.wait = new Infra::CSemaphore( 0 );
    m_state_list.push_back( args );
    m_sem.post();
	args.wait->pend();
	delete args.wait;
	args.wait = NULL;
	return;
}

void CStateCallback::pushMessage( CHeartbeatMonitor::HAState localState, CHeartbeatMonitor::HAState peerState,
                                  TMulticastHeatBeatMsgPtr msg )
{
    HbMsgArgs args;
    args.localState = localState;
    args.peerState = peerState;
    args.msg = msg;
    m_msg_list.push_back( args );
    m_sem.post();
}

void CStateCallback::threadProc()
{
    int num = sysconf( _SC_NPROCESSORS_CONF );
    if( num > 1 ) {
        cpu_set_t mask;
        CPU_ZERO( &mask );
        CPU_SET( 2, &mask );
        int ret = pthread_setaffinity_np( pthread_self(), sizeof( mask ), &mask );
        if( ret != 0 ) {
            DBHALOG_ERROR( "pthread_setaffinity_np failed with error %d, cpu num %d.", ret, num );
        } else {
            DBHALOG_INFO( "set affinity to:%u", 2 );
        }
    }
    while( looping() && !m_exit_flag ) {
        m_sem.pend();
        if( m_exit_flag ) {
            continue;
        }

        if( !m_state_list.empty() ) {
            HbStateArgs args = m_state_list.front();
            m_state_list.pop_front();
            DBHALOG_INFO( "HA state change to %d", args.action );

            m_ha_stat_proc( args.action, args.link );
			if( NULL != args.wait ){
				args.wait->post();
			}
        } else if ( !m_msg_list.empty() ) {
            // 如果心跳消息回调被设置,就交给上层继续处理
            HbMsgArgs args = m_msg_list.front();
            m_msg_list.pop_front();
            if ( !m_msg_proc.empty() ) {
                m_msg_proc( args.localState, args.peerState, args.msg );    
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
class CHeartbeatMonitor::Internal : public Infra::CThread
{
public:
    Internal();
    ~Internal();

	void setJudger( JudgerType type, Judger judger );
	void rmJudger( JudgerType type, Judger judger );
    bool setMulticastAddress( const std::string& ip, int32_t port );
    bool addInterface( const std::string& localIp );
    bool setVirtualIP( const std::string& vif_name, const std::string& if_name, 
		const std::string& vip, const std::string& netmask, const bool& flag );
    bool setTimerPeriod( int32_t interval, int32_t times );
    void setWaitInterval( uint32_t interval );
    void registerMsgCallback( MsgHandlerProc proc );
    void unregisterMsgCallback();
    bool start( GetHbArgsCallback args_proc, HAStateProc state_proc );
    bool stop();

private:
    bool initInputFd();								// 心跳 客户端socket初始化
    bool addMemship( const std::string& ip );		// 把本机加入组播地址
    void timeoutProc( int32_t id );
    int handleInput( int fd );
    void handleMasterRequest( TMulticastHeatBeatMsgPtr req, const std::string& sourceIp );
    void handleSlaveResponse( TMulticastHeatBeatMsgPtr rsp, const std::string& sourceIp );
    void sendSlaveResponse();
    void sendMasterRequest();
    void sendPrepareRequest();
    void sendDeadRequest();
    void sendMultipleHBRequest( TMulticastHeatBeatMsgPtr ptr );
    void changeToSlave();
    void changeToMaster();
    void enableVip();
    void disableVip();
    void arping();

    // 消息过滤、比较相关函数
    // 判断消息是否属于同一个Heartbeat组
    bool filter( TMulticastHeatBeatMsgPtr msg );
    // 在相同状态下，通过比较，返回true则优先成为master
    // 本机和远端状态相同的情况下,调用该函数比较, 返回true优先成为Master
    bool compare( TMulticastHeatBeatMsgPtr local, TMulticastHeatBeatMsgPtr peer, std::stringstream& judgement );
    // 比较两个消息是否来自同一个Slave,true表示相等
    bool isEqual( TMulticastHeatBeatMsgPtr lhs, TMulticastHeatBeatMsgPtr rhs );
    // 获取作为master的对外发布消息包(上层不需要关心消息序号,由心跳模块内部设置消息序列号)
    TMulticastHeatBeatMsgPtr getMasterRequest();
    // 获取作为slave，接收到master消息的回复包(上层不需要关心消息序号,由心跳模块内部设置消息序列号)
    TMulticastHeatBeatMsgPtr getSlaveResponse();
    TMulticastHeatBeatMsgPtr getMessage( bool isRequest );
    //检查状态是否有变化
    void pushState(bool errorFlag=false);

	// 仅用于成主或成备时的状态回调
	void pushStateAndWait( HAAction action );
	
    //增加发送计数
    void addSendTimes( int32_t times = 1 );
    //重置链接计数器
    void resetLinkTimes( std::string source_ip );
    //获得当前链接状态
    bool getLinkState( LinkState& link, bool& curIsAllOffline, bool& preIsAllOffline );
    //检查业务线是否正常,正常返回成功
	bool checkLink(std::string ifName);
    bool checkBusinessLink();
	bool checkHeartbeatLink();
	//保存主机binlog配置文件
	//void saveMasterBinlogConfig(TMulticastHeatBeatMsgPtr local, TMulticastHeatBeatMsgPtr peer);

	//保存备机发到主机的slave sql与sync状态用于运维查询

	//void saveDBSlaveState(TMulticastHeatBeatMsgPtr peer);

	bool putMasterBinlogInfo(int32_t flag);
	//判断DBHA备机是否可以成为主机
	//true可以成为主
	int32_t checkDBHACanBecomeMaster();

	//备机保存主机发送过来的binlog信息，备机只保存在内存
	void saveHeartBeatInfo(TMulticastHeatBeatMsgPtr peer);

private:
    //arp线程，arp发送时间长，影响定时器线程
    void threadProc();
	void deal_short_timer();
	void deal_long_timer();
	// 查看当前slave和主机真正同步的情况
	bool getSlaveSyncState();
	bool canBecomeMasterDirectly();
    bool judge( JudgerType type, std::stringstream& judgement );
	inline const char* strState( int state );
private:
    struct LinkInfo {
        std::string name;		// 链路名称(即接收心跳的本地IP),例如 172.5.5.119/192.168.0.119 等
        int fd;					// 该链路对应的fd
        int32_t times;			// 该链路接收到的次数
        OnlineState curState;	// 当前链路是在线还是离线
        OnlineState preState;	// 当前链路是在线还是离线

        LinkInfo()
        {
            fd = -1;
            times = 0;			//0应该是在线状态，但是在setTimerPeriod会增加m_times次数，相当于初始化times = m_times
            curState = offline;
            preState = offline;
        }
    };

	Infra::CSemaphore m_sem;
	bool m_exit_flag;
	bool m_stop_flag;

    HAAction m_preAction;				// 前一次状态
    std::string m_multicast_ip;         // 组播地址
    int m_multicast_port;               // 组播端口号
    std::map<std::string, LinkInfo> m_interfaces;   // 发送心跳链路
    int m_input_fd;                     // 服务端(接收心跳)socket
    std::string m_vip_if_name;          // 虚IP对应的网卡名,如 bond0:1,用于挂载虚IP: ifconfig bond0:1 172.5.4.10 netmask 255.255.0.0 up
    std::string m_real_if_name;         // 实IP对应网卡名,发送arp广播需要用真实网卡名:  arping -b -U -c 1  -I bond0 172.5.4.10
	std::string m_heartbeat_if_name;    // 实心跳IP对应网卡名,发送arp广播需要用真实网卡名:  arping -b -U -c 1  -I bond0 172.5.4.10
	std::string m_vip;                  // 虚IP
    std::string m_vip_netmask;          // 虚IP子网掩码
    int32_t m_timer_interval;           // 心跳周期
    int32_t m_times;                    // m_times个心跳包收不到就认为超时
    int32_t m_short_period_timer;
    int32_t m_long_period_timer;
    int32_t m_arping_timer;
    int32_t m_arping_interval;
    uint32_t m_send_seq;             // 从prepare状态开始(也包括后面的master状态),本机对外主动发送的心跳序号
    // 该字段有两个功能：
    // 1, master节点用来记录最近一次接收到的slave response消息号,过滤因网卡绑定而收到的重复response.这也个字段也是针对一主一备设计的,不支持一主多备;
    // 2, slave节点 用来记录最近一次接收到的master request消息号,以此作为response报文的序号,并以此过滤因网卡绑定而收到的重复request
    uint32_t m_recv_seq;
	TMulticastHeatBeatMsgPtr m_recv_msg; // 保存最新收到的消息
    uint32_t m_hb_num;               // 统计本机进入prepare状态且成为Master已经发送了多少个心跳
    uint32_t m_wait_interval;        // prepare状态连续N个心跳之后没有收到response,本机将直接进入Master状态
    CHeartbeatWorker* m_worker;
    int32_t m_state;
    MsgHandlerProc m_msg_handler;
    GetHbArgsCallback m_get_hb_args_proc;
    CStateCallback* m_callback;
    CProtoParser m_parser;

	//mysql主备框架主机框架倒掉但是主机mysql服务ok这时备机框架不能立刻成为主
	//需要等待6个超时周期才需要做主备切换
	//防止DBHA服务闪断又恢复的场景
	int32_t m_DBHA_slave_wait_master_time;

	//如果发现备机的mysql服务出现ioerror或者是主备未同步则需要
	//等待的最大超时时间需要等10分钟

	int32_t m_mysql_slave_wait_master_online_time;

	int32_t m_connect_timeout;		//主备业务心跳同时断开的时间
	bool m_single_mode;				//单双机模式
	std::map<JudgerType, std::list<Judger> > m_judgers; ///< 判断规则
};

CHeartbeatMonitor::Internal::Internal() :
    CThread( "arping worker" )
{
    m_multicast_port = 0;
    m_input_fd = -1;
    m_timer_interval = 0;
    m_times = 0;
    m_short_period_timer = 0;
    m_long_period_timer = 0;
    m_arping_timer = 0;
    m_arping_interval = 2 * 1000;
    m_worker = new CHeartbeatWorker;
    m_state = init;
    m_callback = new CStateCallback;
    m_recv_seq = 0;
    m_send_seq = 0;
    m_hb_num = 0;
    m_preAction = NoneAction;

    CProtoParser::setPacketFactory( DS_PROTOCOL_REQ, HAFK_MULTICAST_HEARTBEAT_MESSAGE, &newMulticastHeartBeatMessageRequest );
    CProtoParser::setPacketFactory( DS_PROTOCOL_RES, HAFK_MULTICAST_HEARTBEAT_MESSAGE, &newMulticastHeartBeatMessageResponse );

    m_stop_flag = false;
    m_exit_flag = false;
	m_wait_interval = 0;
	m_DBHA_slave_wait_master_time = 0;
	m_mysql_slave_wait_master_online_time = 0;
	m_connect_timeout = 0;
	m_single_mode = false;

    createThread();
}

CHeartbeatMonitor::Internal::~Internal()
{
    if( m_worker != NULL ) {
        delete m_worker, m_worker = NULL;
    }
    if( m_callback != NULL ) {
        delete m_callback, m_callback = NULL;
    }

    m_exit_flag = true;
    m_sem.post();
    destroyThread();

	std::map<std::string, LinkInfo>::iterator it = m_interfaces.begin();
	for( ;it!=m_interfaces.end(); it++){
		int fd = it->second.fd;
		if(fd!=-1){
			close(fd);
		}
	}

	if(m_input_fd!=-1){
		close(m_input_fd);
	}
}

void CHeartbeatMonitor::Internal::setJudger( JudgerType type, Judger judger )
{
	m_judgers[type].push_back( judger );
	return;
}

void CHeartbeatMonitor::Internal::rmJudger( JudgerType type, Judger judger )
{
	std::map<JudgerType, std::list<Judger> >::iterator it = m_judgers.find( type );
	if( it != m_judgers.end() ){
		std::list<Judger>::iterator judger_it = it->second.begin();
		for( ; judger_it != it->second.end(); ++judger_it ){
			if( *judger_it == judger ){
				it->second.erase( judger_it );
				break;
			}
		}
	}
	return;
}

bool CHeartbeatMonitor::Internal::setMulticastAddress( const std::string& ip, int32_t port )
{
    m_multicast_ip = ip;
    m_multicast_port = port;
    if( initInputFd() ) {
        std::map<std::string, LinkInfo>::iterator it = m_interfaces.begin();
        for( ; it != m_interfaces.end(); ++it ) {
            if( !addMemship( it->first ) ) {
                DBHALOG_ERROR( "add interface(%s) to memship error.", it->first.c_str() );
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}

// 增加组播出口网卡(初始化心跳发送端socket)，多次调用可以实现多网卡路径心跳
bool CHeartbeatMonitor::Internal::addInterface( const std::string& localIp )
{
    if( localIp.empty() ) {
        return false;
    }
    // 不允许重复添加同一地址
    std::map<std::string, LinkInfo>::iterator it = m_interfaces.find( localIp );
    if( it != m_interfaces.end() ) {
        DBHALOG_ERROR( "interface (%s) already exists.",  localIp.c_str() );
        return false;
    }

    // 组播地址否为空说明尚未调用setMulticastAddress,所以先不调用initInputFd和addMemship
    // 待用户调用setMulticastAddress设置组播地址时,其内部也会调用 initInputFd 和 addMemship
    if( m_multicast_port != 0 && !m_multicast_ip.empty() ) {
        if( !initInputFd() ) {
            DBHALOG_ERROR( "init input fd error." );
            return false;
        }

        // 可以先调用addInterface再调用setMulticastAddress,如果这样的话调用这里的addMemship就会因组播地址为空而失败
        if( !addMemship( localIp ) ) {
            DBHALOG_ERROR( "add interface(%s) to memship error.", localIp.c_str() );
            return false;
        }
    }

    int fd = socket( AF_INET, SOCK_DGRAM, 0 );
    if( fd == -1 ) {
        DBHALOG_ERROR( "create socket error, %d.", errno );
        return false;
    }
    int flags = ::fcntl( fd, F_GETFL );
    flags |= O_NONBLOCK;
	if( ::fcntl( fd, F_SETFL, flags ) == -1 ){
		DBHALOG_ERROR( "socket fcntl error, %d.", errno );
		close(fd);
		return false;
	}
    struct sockaddr_in tmp;
    memset( &tmp, 0, sizeof( tmp ) );
    tmp.sin_family = AF_INET;
    tmp.sin_port = 0;
    tmp.sin_addr.s_addr = inet_addr( localIp.c_str() );

    // 禁止组播数据回送，当单台机器测试，需要组播回送，否则自己收不到自己的组播包
    int loop = 0;
#ifdef GTEST_SINGLE_COMPUTER
    loop = 1;
#endif

    int ret = setsockopt( fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof( loop ) );
    if( ret < 0 ) {
        DBHALOG_ERROR( "setsockopt  IP_MULTICAST_LOOP errno %d.", errno );
		close(fd);
        return false;
    }

    /*
    //设置ttl为1跳
    int ttl = 1;
    ret = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl) );
    if( ret < 0 ) {
		DBHALOG_ERROR( "CHeartbeatMonitor::%s %d setsockopt  IP_MULTICAST_TTL errno %d",
					  errno );
		close(fd);
		return false;
	}
	*/

    if( ::bind( fd, ( struct sockaddr * ) &tmp, sizeof( tmp ) ) < 0 ) {
        DBHALOG_ERROR( "bind error:%d.", errno );
		close(fd);
        return false;
    }
    m_interfaces[localIp].fd = fd;
    m_interfaces[localIp].name = localIp;
    DBHALOG_INFO( "add new interface(%s), fd:%d.", localIp.c_str(), fd );
    return true;
}

bool CHeartbeatMonitor::Internal::setVirtualIP( const std::string& vif_name, const std::string& if_name, 
											   const std::string& vip, const std::string& netmask, const bool& flag )
{
    m_vip_if_name = vif_name;
    m_real_if_name = if_name;
    m_vip = vip;
    m_vip_netmask = netmask;
	m_single_mode = flag;
    disableVip();
    return true;
}

bool CHeartbeatMonitor::Internal::setTimerPeriod( int32_t interval, int32_t times )
{
    if( interval <= 0 || times <= 0 ) {
        return false;
    }

    m_timer_interval = interval;
    m_times = times;
    addSendTimes( m_times );
    return true;
}

bool CHeartbeatMonitor::Internal::start( GetHbArgsCallback args_proc, HAStateProc state_proc )
{
    if( args_proc.empty() || state_proc.empty() ) {
        DBHALOG_ERROR( "invalid param!" );
        return false;
    }
	
    m_get_hb_args_proc = args_proc;
    m_state = init;
    m_stop_flag = false;
	
    m_callback->setStateProc( state_proc );
    CHeartbeatWorker::TimeoutProc proc( &Internal::timeoutProc, this );
    // #4352  延迟2个心跳周期再开始计时,确保启动init->prepare的时间 至少比 故障切换时间 多一个心跳周期，如果Master挂掉后立即启动，Slave有足够时间变成Master并发出心跳包
    Infra::CThread::sleep( 2 * m_timer_interval );
    m_long_period_timer = m_worker->setTimer( m_timer_interval * m_times, proc );
    DBHALOG_INFO( "heartbeat monitor start! long timer id(%d).", m_long_period_timer );
    return true;
}

bool CHeartbeatMonitor::Internal::stop()
{
    if( m_long_period_timer ) {
        m_worker->destroyTimer( m_long_period_timer );
        m_long_period_timer = 0;
    }
    if( m_short_period_timer ) {
        m_worker->destroyTimer( m_short_period_timer );
        m_short_period_timer = 0;
    }
	
    m_get_hb_args_proc = NULL;
    m_state = init;
    m_stop_flag = true; //设置线程退出状态,析构时也会设置
	//析构m_callback时会等待线程destory所以此处不用置为空
    //m_callback->setStateProc( NULL );
	
	disableVip();

    DBHALOG_WARN( "stop success."  );
    return true;
}

void CHeartbeatMonitor::Internal::setWaitInterval( uint32_t interval )
{
    m_wait_interval = interval;
}


void CHeartbeatMonitor::Internal::registerMsgCallback( MsgHandlerProc proc )
{
    m_msg_handler = proc;
    m_callback->setMsgProc( m_msg_handler );
    DBHALOG_DEBUG( " message callback function is set." );
}

void CHeartbeatMonitor::Internal::unregisterMsgCallback()
{
    MsgHandlerProc proc = NULL;
    m_msg_handler = proc;
    m_callback->setMsgProc( m_msg_handler );
    DBHALOG_DEBUG( "message callback function is unset." );
}

// 心跳接收端socket初始化
bool CHeartbeatMonitor::Internal::initInputFd()
{
    if( m_multicast_port == 0 || m_multicast_ip.empty() ) {
        return false;
    }
    if( m_input_fd != -1 ) {
        return true;
    }

    m_input_fd = socket( AF_INET, SOCK_DGRAM, 0 );
    if( m_input_fd == -1 ) {
        DBHALOG_ERROR( "create socket error, errno:%d.", errno );
        return false;
    }
    int flags = ::fcntl( m_input_fd, F_GETFL );
    flags |= O_NONBLOCK;
	if( ::fcntl( m_input_fd, F_SETFL, flags ) == -1 ){
		DBHALOG_ERROR( "socket fcntl error,%d", errno );
		return false;
	}

    int on = 1;
    if( setsockopt( m_input_fd, SOL_SOCKET, SO_REUSEADDR, ( const char* )&on, sizeof( on ) ) == -1 ) {
        DBHALOG_ERROR( "reuseaddr failed. error %d", errno);
    }

    //SO_REUSEPORT 在ARM 4.8.3环境下编译不通过，修改成15
	if( setsockopt( m_input_fd, SOL_SOCKET, 15,( const char* )&on, sizeof( on ) ) == -1 ){
		DBHALOG_ERROR( "reuseport failed. error %d", errno );
	}

    struct sockaddr_in tmp;
    memset( &tmp, 0, sizeof( tmp ) );
    tmp.sin_family = AF_INET;
    tmp.sin_port = htons( m_multicast_port );
    tmp.sin_addr.s_addr = inet_addr( m_multicast_ip.c_str() );
    if( ::bind( m_input_fd, ( struct sockaddr * ) &tmp, sizeof( tmp ) ) < 0 ) {
        DBHALOG_ERROR( "bind error:%d", errno );
        return false;
    }
	
    CHeartbeatWorker::ReadProc proc( &Internal::handleInput, this );
    if( !m_worker->registerReadEvent( m_input_fd, proc ) ) {
        return false;
    }
    return true;
}

// 把本机加入组播地址
bool CHeartbeatMonitor::Internal::addMemship( const std::string& ip )
{
    struct ip_mreq mreq;
    bzero( &mreq, sizeof( struct ip_mreq ) );
    mreq.imr_multiaddr.s_addr = inet_addr( m_multicast_ip.c_str() );
    mreq.imr_interface.s_addr = inet_addr( ip.c_str() );
    if( ::setsockopt( m_input_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof( struct ip_mreq ) ) == -1 ) {
        DBHALOG_ERROR( "set IP_ADD_MEMBERSHIP failed! interface:%s, multiaddr:%s, error[%s]:%d",
                      ip.c_str(), m_multicast_ip.c_str(), strerror( errno ), errno );
        return false;
    }
    return true;
}

void CHeartbeatMonitor::Internal::deal_short_timer()
{
	if( m_state == prepare ) {
		// m_hb_num是心跳次数，其间隔是m_timer_interval毫秒，而m_wait_interval的单位是秒
		if( m_hb_num > ( m_wait_interval * 1000 / m_timer_interval ) ) {
			// 选举超时，尝试自己成主
			std::stringstream judgement;
			judgement << "[PASS] wait for peer message timeout("<< m_wait_interval <<"s).\n";
			if( judge( BECOME_MASTER, judgement ) ){
				ELECTION_LOG( m_state, master, BECOME_MASTER, judgement );
				changeToMaster();
				m_hb_num = 0;
			}else{
				ELECTION_LOG( m_state, dead, BECOME_MASTER, judgement );
				// 无法成为主，pushState(true)将会通知上层退出进程
				pushState( true );
			}
		} else {
			// 未超时，继续发送选举消息
			sendPrepareRequest();
			m_hb_num++;
		}
	} else if( m_state == master ) {
		// 检查链路情况，通知上层
		pushState();
		std::stringstream judgement;
		if( judge( BECOME_MASTER, judgement ) ){
			// 仍满足成主规则，发消息维持master地位
			sendMasterRequest();
		}else{
			ELECTION_LOG( m_state, dead, BECOME_MASTER, judgement );
			pushState( true );
		}
	} else if( m_state == slave ) {
		// 检查链路情况，通知上层
		pushState();
	} else {
		//ASSERT_PRINT( 0 );
		DBHALOG_WARN( "badstate[%d] short timer timeout.", m_state );
		m_worker->destroyTimer( m_short_period_timer );
		m_short_period_timer = 0;
	}
}

void CHeartbeatMonitor::Internal::deal_long_timer()
{
	// 长周期定时器超时，说明一段时间内没有收到对端的消息了，开始判断自身能否成主
	DBHALOG_DEBUG( "long period timer timeout!" );
	// 重置之前收到的消息，避免使用之前的消息内容进行比较
	m_recv_msg.reset();
	
	CHeartbeatWorker::TimeoutProc proc( &Internal::timeoutProc, this );
	std::stringstream judgement; // 记录决策信息，进入该函数，第一条肯定是等待对端消息超时了
	judgement << "[PASS] wait for peer message timeout(" << m_timer_interval * m_times << "ms).\n";
	if( m_state == init ){
		if( judge( BECOME_MASTER, judgement ) ){
			if( judge( FAST_BECOME_MASTER, judgement ) ){
				// 满足快速成主的条件，则跳过选举
				ELECTION_LOG( m_state, master, FAST_BECOME_MASTER, judgement );
				changeToMaster();
				return;
			}
			ELECTION_LOG( m_state, prepare, BECOME_MASTER, judgement );
			m_state = prepare;
			sendPrepareRequest();
			m_hb_num++; 
			m_short_period_timer = m_worker->setTimer( m_timer_interval, proc );
			m_worker->destroyTimer( m_long_period_timer );
			m_long_period_timer = 0;
		}else{
			// 无法成主，则维持原来的状态，周期性检查，避免出现双备
			ELECTION_LOG( m_state, init, BECOME_MASTER, judgement );
			// 超时无法成主也不能直接成备，但必须发送消息能让对端感知到本端的信息
			sendSlaveResponse();
		}
	} else if ( m_state == slave ) {
		// 记录m_times没有收到对端消息,认为主已死，判断自己能否成主
		addSendTimes( m_times );
		if( judge( BECOME_MASTER, judgement ) ){
			ELECTION_LOG( m_state, master, BECOME_MASTER, judgement );
			changeToMaster();
		}else{
			// 发送一个slave包，继续等待，看后续是否能成为主
			ELECTION_LOG( m_state, slave, BECOME_MASTER, judgement );
			sendSlaveResponse();
			DBHALOG_INFO( "waiting to become master." );
			pushState();
		}
	}else {
		DBHALOG_WARN( "badstate[%d] long timer timeout.", m_state );
		m_short_period_timer = m_worker->setTimer( m_timer_interval, proc );
		m_worker->destroyTimer( m_long_period_timer );
		m_long_period_timer = 0;
	}
}

void CHeartbeatMonitor::Internal::timeoutProc( int32_t id )
{
//	DBHALOG_DEBUG("timer id : %d, last_state_change_timer : %d"
//			"short_period_timer : %d, long_period_timer : %d, arping_timer : %d",
//			 id, m_last_state_change_timer, m_short_period_timer,
//			m_long_period_timer, m_arping_timer);
    if( id == m_short_period_timer ) {
		deal_short_timer();
	} else if( id == m_long_period_timer ) {
		deal_long_timer();
	} else if( id == m_arping_timer ) {
        if( m_arping_interval * 2 <= 60 * 1000 ) {
            m_arping_interval *= 2;
            m_worker->destroyTimer( m_arping_timer );
            CHeartbeatWorker::TimeoutProc proc( &Internal::timeoutProc, this );
            m_arping_timer = m_worker->setTimer( m_arping_interval, proc );
        }
        m_sem.post();
    }
}

int  CHeartbeatMonitor::Internal::handleInput( int fd )
{
    ASSERT_ABORT( fd == m_input_fd );
    sockaddr_in src_addr;
    memset( &src_addr, 0, sizeof( sockaddr ) );
    // 如果不赋初始值,获取到的远端IP是0.0.0.0
    socklen_t addr_len = sizeof( sockaddr );
    int ret = recvfrom( fd, m_parser.getBuffer(), m_parser.getLength(), 0, ( struct sockaddr * ) &src_addr, &addr_len );
    if( ret <= 0 ) {
        DBHALOG_ERROR( "read error,errno:%d!", errno );
        return -1;
    }
    struct in_addr ia;
    ia.s_addr = src_addr.sin_addr.s_addr;
    std::string source_ip = inet_ntoa( ia );

    int pkt_num = m_parser.putDataLen( ret );
    if( pkt_num < 0 ) {
        DBHALOG_ERROR( "failed to parse packet!" );
        return -1;  
    }
    TPacketBasePtr pkt_base = m_parser.getPacket();
    // pkt_base->printPacket();
    TMulticastHeatBeatMsgPtr pkt = dynamic_pointer_cast<CMulticastHeartBeatMessage>( pkt_base );
    if( NULL == pkt ) {
        DBHALOG_ERROR( "failed to convert message to CMulticastHeartBeatMessage!" );
        return -1;
    }

    // 由于需要分别检查每条链路的健康状态,消息处理流程改为:
    // 1, 过滤不是同一集群的消息; 2,重置消息所经链路的定时器; 3,根据消息序号过滤已处理过的消息; 4,处理新达到的消息
    // step 1: 过滤不是同一集群的消息
    if( !filter( pkt ) ) {
        return -2;
    }

    //单台机器测试时，过滤掉和自己IP一样的组播包
    std::map<std::string, LinkInfo>::iterator it = m_interfaces.begin();
    for( ; it != m_interfaces.end(); ++it ) {
        if( it->first == source_ip ) {
            return -1;
        }
    }

    // step 2: 重置消息所经链路的定时器
    // 根据fd从map中查找本机IP,然后根据IP重置定时器
    resetLinkTimes( source_ip );

    // step 3 : 根据消息序号过滤已处理过的消息
    // 对于机器重启,序号重新开始的问题：
    // slave 重启: slave回复的消息号与其接收到的request消息号是一致的,由于master未重启,所以slave发送的序号和master一致,不会重新开始
    // master重启: 如slave检测到master dead且自己成为master,其已接收序号会清零;如slave未检测到master dead,会符合 pkt_base->getSequence() < m_recv_seq 的条件
    //			   然后重新更新序号
    // 当对端跳过选举直接成备时，发送的消息序号是0，该消息不能过滤，否则可能出现双备
    if( 0 < m_recv_seq && pkt_base->getSequence() == m_recv_seq ) {
        // 过滤已经处理过的消息
        // DBHALOG_DEBUG("CHeartbeatMonitor::%s %d message(%u) has received from other link, ignore it!", __FUNCTION__,__LINE__,m_recv_seq);
        return -1;
    }

	if( pkt_base->getSequence() < m_recv_seq ) {
        // 可能是由于对端重启等原因,序号从0开始重新发送
        DBHALOG_WARN( "current msg seq(%u) < last msg seq(%u)!", pkt_base->getSequence(), m_recv_seq );
    }

    uint8_t pkt_type = pkt->getPktType();
    if( pkt_type == DS_PROTOCOL_REQ ) {
        handleMasterRequest( pkt, source_ip );
    } else if( pkt_type == DS_PROTOCOL_RES ) {
        handleSlaveResponse( pkt, source_ip );
    } else {
        DBHALOG_ERROR( "receive error message!"  );
        assert( 0 );
    }
	return 0;
}

// true表示本地做主机；false表示对端做主机
bool CHeartbeatMonitor::Internal::compare( TMulticastHeatBeatMsgPtr local, TMulticastHeatBeatMsgPtr peer, std::stringstream& judgement )
{
	if( NULL == local.get() ){
		DBHALOG_ERROR( "TMulticastHeatBeatMsgPtr local is NULL!" );
		return false;
	}

    int32_t local_current_state = local->getCurrentState();
    int32_t local_history_state = local->getUsedState();
    int32_t local_mater_timestamp = local->getMasterTimeStamp();
    std::string local_ip = local->getLocalIp();
	//int32_t local_mysql_state = local->getMysqlMode();

	if( NULL == peer.get() ){
		DBHALOG_ERROR( "TMulticastHeatBeatMsgPtr peer is NULL!" );
		return false;
	}
    int32_t peer_current_state = peer->getCurrentState();
    int32_t peer_history_state = peer->getUsedState();
    int32_t peer_mater_timestamp = peer->getMasterTimeStamp();
    std::string peer_ip = peer->getLocalIp();
	//int32_t peer_mysql_state = peer->getMysqlMode();

	/* deal_short_timer()--judge()---compare()去掉注释防止一直打印相同信息
	 * DBHALOG_INFO( "local state : %d, local history state : %d, local master timestamp : %d, local ip : %s"
                "peer state : %d, peer history state : %d, peer master timestamp : %d, peer ip : %s",
                local_current_state, local_history_state, local_mater_timestamp, local_ip.c_str(),
                peer_current_state, peer_history_state, peer_mater_timestamp, peer_ip.c_str() );
     */
	
	// 对端已经为主，则本端不能成为主
	if( master == peer_current_state ){
		judgement << "[FAIL] peer current state is master.\n";
		return false;
	}

	// 本端已经是主，则继续为主
	// 对端已经是备，则本端能成主
	if( master == local_current_state 
		|| slave == peer_current_state ){
		judgement << "[PASS] local current state is " << strState( local_current_state ) 
			      << ", peer current state is " << strState( peer_current_state ) << ".\n";
		return true;
	}

	// #29974
	// 收到消息时，对端是init，说明对端无法成主
	if( init == peer_current_state ){
		if( master == peer_history_state 
			&& peer_mater_timestamp > local_mater_timestamp ){
			// 对端历史是主，且成主时间晚于本端，说明对端可能存在更新的数据，本端不能升主，需要人工检查恢复
			judgement << "[FAIL] peer history state is master, it may has lastest data, please check.\n";
			return false;
		}else{
			judgement << "[PASS] peer doesn't have lastest data, local current state is " 
				      << strState( local_current_state ) << ".\n";
			return true;
		}
	}
	
	// 两端都能成主，则根据以下规则，确定一个主
    bool ret = true;
    do{
		// 历史状态,本机是主,远端是备:本机直接做主
        if( ( master == local_history_state ) && ( master != peer_history_state ) ) {
			judgement << "[PASS] local history state is master, and peer history state is not master.\n";
            break;
        }
		
        // 历史状态,本机是备,远端是主：本机成为备机
        if( ( master != local_history_state ) && ( master == peer_history_state ) ) {
			judgement << "[FAIL] local history state is not master, and peer history state is master.\n";
            ret = false;
            break;
        }
        // 历史状态,两端都是主:比较两台机器上一次成为主的时间,时间晚的那个机器继续做主
        if( master == local_history_state  && master == peer_history_state ){
			if( local_mater_timestamp > peer_mater_timestamp ){
				judgement << "[PASS] both history state are master, but local timestamp is larger.\n";
				break;
			}else if( local_mater_timestamp < peer_mater_timestamp ){
				judgement << "[FAIL] both history state are master, but peer timestamp is larger.\n";
				ret = false;
				break;
			}else{
				// 可能都是0,例如升级后初始值为0
				DBHALOG_WARN( "timestamp of master is the same: %d\n", peer_mater_timestamp );
			}
        }
        // IP字符串较大的成为主机
        if( local_ip < peer_ip ) {
			judgement << "[FAIL] local ip is less than the peer ip.\n";
            ret = false;
            break;
        }
		judgement << "[PASS] local ip is greater than the peer ip.\n";
    }while( 0 );

    return ret;
}

// 当不备机切换为主机的时候，需要记录当前主机的位置信息，flag=1表示合法成为主机，0表示强制成为主机
//bool CHeartbeatMonitor::Internal::putBinlogInfo(int32_t flag)
//{
//	std::string changeMasterInfo = "";
//	bool ret = CommonFun::getExecuteScriptValue(CDBHAConfig::instance()->getBinlogInfoScript(), 
//		"",changeMasterInfo);
//	if (!ret)
//	{
//		DBHALOG_ERROR("set master binlog failed");
//		return false;
//	}
//
//	if (!gHAConfig.setChangeMasterInfo(changeMasterInfo,flag))
//	{
//		DBHALOG_ERROR("set master binlog failed into memory");
//		return false;
//	}
//	return true;
//}

bool CHeartbeatMonitor::Internal::getSlaveSyncState()
{
	// 1表示需要从主机获取信息进行匹配；0则不需要
	std::string master_ip="";
	if (g_DBHAConfig->getCheckSyncFlag())
		master_ip = gHAConfig.getPeerBusinessIP();

	std::string retResult="";
	bool ret = CommonFun::getExecuteScriptValue(g_DBHAConfig->getSlaveStateScript(), master_ip, retResult);
	if (!ret){
		DBHALOG_ERROR("slave is not synced with master.");
		return false;
	}

	DBHALOG_INFO("slave is synced with master.");
	return true;
}

/* 1表示彻底同步上了，0表示未同步完全但已经超时了，-1表示还需要等待*/
int32_t CHeartbeatMonitor::Internal::checkDBHACanBecomeMaster()
{
	//扩容状态，备机需等待数据同步
	if( g_DBHAConfig->serviceInExpandMode() )
		return waitingForSync;

	// 获取当前备机的同步情况,完全同步上
	if (getSlaveSyncState())
		return totallySync;

	// 未完全同步上，则等待6次回调，大概10s
	if (m_DBHA_slave_wait_master_time >= g_DBHAConfig->getMaxSlaveSyncWithMasterNum()){
		if( canBecomeMasterDirectly() ){
			DBHALOG_INFO("change to master directly!");
			return totallySync;
		}else{
			return timeoutForSync;
		}
	}

	m_DBHA_slave_wait_master_time++;
	DBHALOG_WARN("m_DBHA_slave_wait_master_time:%d,m_DBHA_slave_wait_master_time:%d",
		m_DBHA_slave_wait_master_time,m_DBHA_slave_wait_master_time);
	return waitingForSync;
}

bool CHeartbeatMonitor::Internal::canBecomeMasterDirectly()
{
	//检测本地的同步状态
	std::string slaveInfo="";
	bool ret = CommonFun::getExecuteScriptValue(g_DBHAConfig->getCheckSyncState(), "", slaveInfo);
	if( !ret || slaveInfo.empty() ) {
		DBHALOG_ERROR( "check slave sync state failed." );
		return false;
	}

	std::string binlogName;
	int32_t binlogPos;
	ret = CommonFun::setBinlogInfo(slaveInfo, binlogName, binlogPos);
	if( !ret ) {
		DBHALOG_ERROR( "set binlog info failed,%s.", slaveInfo.c_str());
		return false;
	}

	//比较binlog的差距，目前一个binlog文件大小为1G，最多只允许相差1-2个binlog文件的差距
	int masterIndex = 0;
	int retS = sscanf(gHeartBeatBinlogInfo.masterCurrentBinlogName.c_str(), "%*[^.].%d", &masterIndex);
	if( retS != 1 ){
		DBHALOG_ERROR( "parser master index failed, binlog:%s, %u, %d.", gHeartBeatBinlogInfo.masterCurrentBinlogName.c_str(), masterIndex, retS);
		return false;
	}

	int slaveIndex = 0;
	if( sscanf(binlogName.c_str(), "%*[^.].%d", &slaveIndex) != 1 ){
		DBHALOG_ERROR( "parser master index failed, binlog:%s, %u.", binlogName.c_str(), slaveIndex);
		return false;
	}

	if( masterIndex - slaveIndex > g_DBHAConfig->getBinlogFileDelayNum() ){
		DBHALOG_ERROR( "binlog num different:%d,%d,%d.", masterIndex, slaveIndex, g_DBHAConfig->getBinlogFileDelayNum());
		return false;
	}

	return true;
}

void CHeartbeatMonitor::Internal::saveHeartBeatInfo(TMulticastHeatBeatMsgPtr peer)
{
	std::string master_init_log_name = peer->getMasterInitBinlogName();
	int32_t master_init_log_pos = peer->getMasterInitBinlogPos();
	std::string change_master_log_name = peer->getMasterBinLogName();
	int32_t change_master_log_pos = peer->getMasterBinlogPos();
	int32_t change_master_flag = peer->getMasterChangeFlag();

	gHeartBeatBinlogInfo.masterCurrentBinlogName = peer->getMasterCurrentBinLogName();
	gHeartBeatBinlogInfo.masterCurrentBinlogPos = peer->getMasterCurrentBinlogPos();
	gHeartBeatBinlogInfo.masterCurrentTime = peer->getMasterCurrentBinlogTime();

	DBHALOG_DEBUG("master_init_log_name:%s,master_init_log_pos:%d,change_master_log_name:%s,"
					"change_master_log_pos:%d,change_master_flag:%d,masterCurrentBinlogName:%s,masterCurrentTime:%llu",
					master_init_log_name.c_str(),master_init_log_pos,change_master_log_name.c_str(),
					change_master_log_pos,change_master_flag,gHeartBeatBinlogInfo.masterCurrentBinlogName.c_str(),
					UINT64_CAST(gHeartBeatBinlogInfo.masterCurrentTime));

	if (master_init_log_name != gHeartBeatBinlogInfo.masterInitBinlogName)
	{
		gHeartBeatBinlogInfo.masterInitBinlogName = master_init_log_name;
	}

	if (master_init_log_pos!= gHeartBeatBinlogInfo.masterInitBinlogPos )
	{
		gHeartBeatBinlogInfo.masterInitBinlogPos = master_init_log_pos;
	}

	if(change_master_flag != gHeartBeatBinlogInfo.changeMasterFlag )
	{
		gHeartBeatBinlogInfo.changeMasterFlag = change_master_flag;
	}

	if (change_master_log_name != gHeartBeatBinlogInfo.changeMasterBinlogName )
	{
		gHeartBeatBinlogInfo.changeMasterBinlogName = change_master_log_name;
	}

	if(change_master_log_pos != gHeartBeatBinlogInfo.changeMasterBinlogPos)
	{
		gHeartBeatBinlogInfo.changeMasterBinlogPos = change_master_log_pos;
	}
}

void CHeartbeatMonitor::Internal::handleMasterRequest( TMulticastHeatBeatMsgPtr req, const std::string &sourceIp )
{
    // 记录收到报文的序号,以便回复、过滤重复消息
	//获取主机发过来的binlog文件名与pos位置
	//TMulticastHeatBeatMsgPtr local_ptr = getMasterRequest();
	//saveMasterBinlogConfig(local_ptr,req);
	saveHeartBeatInfo(req);
    m_recv_seq = req->getSequence();
	m_recv_msg = req; // 保存请求内容，用于判断能否成master
	int8_t remote_state = m_recv_msg->getCurrentState();
	// 只有prepare和master才会发送该请求。
	ASSERT_ABORT( prepare == remote_state || master == remote_state || dead == remote_state);
	DBHALOG_DEBUG( "receive %s request.", prepare == remote_state ? "prepare" :  master == remote_state ? "master": "dead");

	std::stringstream judgement;
    switch( m_state ) {
        case init:  // fallthrough, 在init和prepare阶段时收到对端信息的处理逻辑是一样的
        case prepare:
			if( master == remote_state ){
				// 对端已经是master，则本端直接变slave,跳过其他判断
				judgement << "[FAIL] peer current state is master.\n";
				ELECTION_LOG( m_state, slave, BECOME_MASTER, judgement );
				changeToSlave();
			}else{
				// 对端正在prepare阶段，判断自己能否成master。判断条件保证两端只有一端能成主
				judgement << "[PASS] peer current state is prepare.\n";
				if( judge( BECOME_MASTER, judgement ) ){
					ELECTION_LOG( m_state, master, BECOME_MASTER, judgement );
					changeToMaster();
				}else{
					// 当对端处于prepare时，必然已经满足了成主条件，可以本端可以放心成备
					ELECTION_LOG( m_state, slave, BECOME_MASTER, judgement );
					changeToSlave();
				}
			}
        	break;
        case master:
            if( master == remote_state ){
				// 双主，本端直接退出
				// TODO: 2018.9.25, 可以新增规则判断谁退出，比如让拥有最新数据的一端维持主
				judgement << "[FAIL] peer current state is master.\n";
				ELECTION_LOG( m_state, dead, BECOME_MASTER, judgement );
				//sendMasterRequest();  //发送master报文给对端
                sendDeadRequest(); //发送dead至对端，通知对端重启
				pushState( true );
			}else{
				// 对端为prepare时，如果对端能收到本端的消息，则会自动成备
                // 对端为dead,则重启，本端也重启
                if( dead == remote_state){
                    judgement << "[FAIL] peer current state is restart.\n";
				    ELECTION_LOG( m_state, dead, BECOME_MASTER, judgement );
                    // 出现双主，对端已重启，本端直接退出
				    pushState( true );
                }
	            DBHALOG_WARN( "current state is master, but receive request msg(peer state: %d)", remote_state );
			}
            break;
        case slave:
			if( master == remote_state ){
				// 接受对端领导，重置定时器
				m_worker->resetTimer( m_long_period_timer );
	            sendSlaveResponse();
	            addSendTimes();

	            if( !m_msg_handler.empty() ){
	                m_callback->pushMessage( slave, master, req );
	            }
			}else{
				// 对端正在prepare阶段，判断自己能否成master。判断条件保证两端只有一端能成主
				judgement << "[PASS] peer current state is prepare.\n";
				if( judge( BECOME_MASTER, judgement ) ){
					ELECTION_LOG( m_state, master, BECOME_MASTER, judgement );
					changeToMaster();
				}
			}
            break;
        default:
            break;
    }
}

void CHeartbeatMonitor::Internal::handleSlaveResponse( TMulticastHeatBeatMsgPtr rsp, const std::string& sourceIp )
{
	DBHALOG_DEBUG( "receive slave response." );
    // 记录收到报文的序号,以便回复、过滤重复消息
	//获取备机端发过来的slave sql状态与sync状态
	//saveDBSlaveState(rsp);
	int8_t remote_state = rsp->getCurrentState();
	// 只有init或者slave状态才会发送该消息
	ASSERT_ABORT( slave == remote_state || init == remote_state );
	m_recv_seq = rsp->getSequence();
	m_recv_msg = rsp;
    saveHeartBeatInfo(rsp);
	std::stringstream judgement;
	if( master != m_state ){
		// 收到SlaveResponse说明对端无法成主，但仍需判断本端是否能成master
		judgement << "[PASS] peer can't be master, it's current state is " << strState( remote_state ) << ".\n";
		if( judge( BECOME_MASTER, judgement ) ){
			ELECTION_LOG( m_state, master, BECOME_MASTER, judgement );
			changeToMaster();
		}else{
			// 本端也无法成master，如果本端成slave，那就出现了双slave
			// 所以让本端重启吧，看看有没有奇迹发生。
			ELECTION_LOG( m_state, dead, BECOME_MASTER, judgement );
			pushState( true );
		}
	}

	if( master == m_state ){
		// 备机没有离线，则将消息回调到上层
		if( m_preAction != slaveOffline && !m_msg_handler.empty() ){
			m_callback->pushMessage( master, slave, rsp );
		}
	}
    return;
}

void CHeartbeatMonitor::Internal::sendSlaveResponse()
{
	DBHALOG_DEBUG( "send slave response(%u)", m_recv_seq );
    TMulticastHeatBeatMsgPtr ptr = getSlaveResponse();
    if( ptr == NULL ) {
        DBHALOG_ERROR( "get slave response error!" );
        return;
    }

    // 消息序列号由心跳模块内部维护
    ptr->setSequence( m_recv_seq );

    const char * pkt = ptr->getData();
    uint32_t pktLen = ptr->getDataLen();

    // 待长度确定之后再print
    // ptr->printPacket();

    struct sockaddr_in remote;
    uint32_t socklen = sizeof( struct sockaddr_in );
    memset( &remote, 0, socklen );
    remote.sin_family = AF_INET;
    remote.sin_port = htons( m_multicast_port );
    inet_pton( AF_INET, m_multicast_ip.c_str(), &remote.sin_addr );

    std::map<std::string, LinkInfo>::iterator it = m_interfaces.begin();
    for( ; it != m_interfaces.end(); ++it ) {
        int fd = it->second.fd;
        DBHALOG_DEBUG( "send response via(%s)", it->first.c_str());
        if( ::sendto( fd, pkt, pktLen, 0, ( const sockaddr* ) &remote, socklen ) <= 0 ) {
            DBHALOG_ERROR( "send slave response via(%s) error, errno:%d", it->first.c_str(), errno );
            // 不return,通过其他链路继续发送
        }
    }
}

void CHeartbeatMonitor::Internal::sendPrepareRequest()
{
//	DBHALOG_DEBUG( "send prepare request(%u)",  m_send_seq);
    TMulticastHeatBeatMsgPtr ptr = getMasterRequest();
    if( ptr == NULL ) {
        DBHALOG_ERROR( "failed to get master request!" );
        return;
    }
    // 上层不感知prepare状态,所以在这里将其设置成prepare状态
    ptr->setCurrentState( prepare );
    sendMultipleHBRequest( ptr );
    addSendTimes();
}

void CHeartbeatMonitor::Internal::sendMasterRequest()
{
//	DBHALOG_DEBUG( "send master request(%u)",  m_send_seq);
    TMulticastHeatBeatMsgPtr ptr = getMasterRequest();
    if( ptr == NULL ) {
        DBHALOG_ERROR( "failed to get master request!" );
        return;
    }
    sendMultipleHBRequest( ptr );
    addSendTimes();
}

void CHeartbeatMonitor::Internal::sendDeadRequest()
{
    TMulticastHeatBeatMsgPtr ptr = getMasterRequest();
    if( ptr == NULL ) {
        DBHALOG_ERROR( "failed to get master request!" );
        return;
    }

    ptr->setCurrentState( dead );
    sendMultipleHBRequest( ptr );
    addSendTimes();
}

void CHeartbeatMonitor::Internal::sendMultipleHBRequest(TMulticastHeatBeatMsgPtr ptr)
{
	ptr->setSequence(++m_send_seq);  // 序号从1开始,因为m_recv_seq初始值也是0,这样序号为0的包会被当做已经接收的包而过滤掉

	const char * pkt = ptr->getData();
	uint32_t pktLen = ptr->getDataLen();
	struct sockaddr_in remote;
	uint32_t socklen = sizeof(struct sockaddr_in);
	memset(&remote, 0, socklen);
	remote.sin_family = AF_INET;
	remote.sin_port = htons(m_multicast_port);
	inet_pton(AF_INET, m_multicast_ip.c_str(), &remote.sin_addr);
	std::map<std::string, LinkInfo>::iterator it = m_interfaces.begin();
	for (; it != m_interfaces.end(); ++it) {
		int fd = it->second.fd;
		// DBHALOG_DEBUG( "send request via(%s)",  it->first.c_str());
		if (::sendto(fd, pkt, pktLen, 0, (const sockaddr*)&remote, socklen) <= 0) {
			DBHALOG_ERROR("send request via(%s) error, errno:%d", it->first.c_str(), errno);
		}
	}
}

void CHeartbeatMonitor::Internal::changeToSlave()
{
	DBHALOG_WARN( "HA status change to Slave!" );
   
    m_preAction = NoneAction;
    m_state = slave;
	// 成为slave之后，清空本机主动发送序列号。slave response报文的序号就是自己刚刚收到的request序号。
    m_send_seq = 0;
	m_connect_timeout = 0;
    sendSlaveResponse();
	
	// 通知上层并等待上层切换到slave状态
	pushStateAndWait( becomeSlave );

	// 状态完整切换后，设置定时器
	CHeartbeatWorker::TimeoutProc proc( &Internal::timeoutProc, this );
    if( m_long_period_timer == 0 ) {
        m_long_period_timer = m_worker->setTimer( m_timer_interval * m_times, proc );
    } else {
        m_worker->resetTimer( m_long_period_timer );
    }
	
    if( m_short_period_timer != 0 ) {
        m_worker->resetTimer( m_short_period_timer );
    } else {
        m_short_period_timer = m_worker->setTimer( m_timer_interval, proc );
    }

	return;
}

void CHeartbeatMonitor::Internal::changeToMaster()
{
    DBHALOG_WARN( "HA status change to Master!"  );
    m_preAction = NoneAction;
    m_state = master;

	// 在通知上层前，先启动虚IP，减少上层切换耗时长时，对端也升主的风险
    enableVip();
    arping();

	// 先发送一个master消息，确定master地位，减少上层切换耗时长时，对端也升主的风险
	m_recv_seq = 0; 
	sendMasterRequest();
	
	// #29889，通知上层并等待上层切换到master状态
	// NOTE:如果上层切换时间大于主备选举超时时间（当前默认10分钟），那可能出现双主
	pushStateAndWait( becomeMaster );
	
	// 因为上层切换耗时可能有点长，所以切换完成后立即发一个master消息
	sendMasterRequest();
	
	// 状态完整切换后，设置定时器
    CHeartbeatWorker::TimeoutProc proc( &Internal::timeoutProc, this );
    m_arping_timer = m_worker->setTimer( m_arping_interval, proc );
    if( m_short_period_timer != 0 ) {
        m_worker->resetTimer( m_short_period_timer );
    } else {
        m_short_period_timer = m_worker->setTimer( m_timer_interval, proc );
    }

    if( m_long_period_timer != 0 ) {
        m_worker->destroyTimer( m_long_period_timer );
        m_long_period_timer = 0;
    }
	
	return;
}

void CHeartbeatMonitor::Internal::enableVip()
{
	//单机模式下DBHA设置的虚IP仍然起起来
    if( m_vip_if_name.empty() || m_vip.empty() || m_vip_netmask.empty() ) {
        DBHALOG_ERROR( "vip if name:%s,vip:%s,netmask:%s is error!", 
                     m_vip_if_name.c_str(), m_vip.c_str(), m_vip_netmask.c_str() );
        return;
    }

    char command[512] = { 0 };
    if( snprintf( command, sizeof( command ) / sizeof( command[0] ), "ifconfig %s %s netmask %s up",
                  m_vip_if_name.c_str(), m_vip.c_str(), m_vip_netmask.c_str() ) < 0 ) {
        DBHALOG_ERROR( "snprintf failed!" );
        return;
    }

    std::stringstream ret_value;
    int32_t ret = Dahua::EFS::os::shell( &ret_value, std::string( command ) );
	if ( ret ) {
		DBHALOG_ERROR( "command: %s falied:%s!", command, ret_value.str().c_str() );
	}
	return;
}

void CHeartbeatMonitor::Internal::disableVip()
{
    if( m_vip_if_name.empty() ) {
        DBHALOG_ERROR( "vip if name:%s is empty!", m_vip_if_name.c_str() );
        return;
    }
    char command[512] = { 0 };
    if( snprintf( command, sizeof( command ) / sizeof( command[0] ), "ifconfig %s down", m_vip_if_name.c_str() ) < 0 ) {
        DBHALOG_ERROR( "snprintf failed!" );
        return;
    }
    DBHALOG_INFO( "command: %s!", command );
    Dahua::EFS::os::shell( NULL, std::string( command ) );
}

void CHeartbeatMonitor::Internal::arping()
{
	/*
	 * arping -b -U -c 1  -I em1 172.5.4.20
	 * -U 免费ARP，不希望收到回应，只希望是起宣告作用
	 * -A ARP回复模式，需要回复
	 * -w 增加arp命令超时时间
	 */
    if( m_real_if_name.empty() || m_vip.empty() ) {
        return;
    }
    char command[512] = { 0 };
    if( snprintf( command, sizeof( command ) / sizeof( command[0] ), "arping -b -U -c %d -I %s %s -w 10", 2,
                  m_real_if_name.c_str(), m_vip.c_str() ) < 0 ) {
        DBHALOG_ERROR( "snprintf failed!" );
        return;
    }
    DBHALOG_DEBUG( "command: %s!", command );
    Dahua::EFS::os::shell( NULL, std::string( command ) );
}

bool CHeartbeatMonitor::Internal::judge( JudgerType type, std::stringstream& judgement )
{
	// 1.业务线必须正常
	if( BECOME_MASTER == type ){
		if( !checkBusinessLink() ){
			//已经是主或已经是备状态的情况下不检测业务线状态
			if (m_state == master){
				judgement << "[WARN]business link is abnormal but skip it!\n";
			}else{
				judgement << "[FAIL] business link is abnormal.\n";
				return false;
			}
		}else{
			judgement << "[PASS] business link is normal.\n";
		}
	}

	// 2.回调上层注册的判断函数
	std::map<JudgerType, std::list<Judger> >::iterator it = m_judgers.find( type );
	if( it != m_judgers.end() ){
		std::list<Judger>::iterator judger_it = it->second.begin();
		for( ; judger_it != it->second.end(); ++judger_it ){
			if( !(*judger_it)( judgement ) ){
				return false;
			}
		}
	}
	
	// 3.和对端比较，谁能成为主
	if( BECOME_MASTER == type ){
		if( NULL != m_recv_msg.get() ){
			TMulticastHeatBeatMsgPtr local = getMasterRequest();
			TMulticastHeatBeatMsgPtr remote = m_recv_msg;
			m_recv_msg.reset(); // 只与最近收到的消息比较一次
			if( !compare( local, remote, judgement ) ){
				return false;
			} 
		}
	}

	return true;
}

const char* CHeartbeatMonitor::Internal::strState( int state )
{
	switch( state ){
		case init:
			return "init";
		case prepare:
			return "prepare";
		case slave:
			return "slave";
		case master:
			return "master";
		case empty:
			return "empty";
		case dead:
			return "dead";
		default:
			ASSERT_ABORT( 0 );
			break;
	}
	return NULL;
}

// [ fieldName : fieldValue : count ]
std::map< std::string, std::map<std::string, uint16_t> > hb_filter_counter;
const uint16_t FILTER_PRINT_INTERVAL = 4 * 2 * 60; // 一套主备MDS每秒发4个组播包，2分钟打印一次


#define COMPARE_STRING_FIELD(fieldName, localValue, peerValue) { \
        if ( (!localValue.empty() && !peerValue.empty()) && (localValue != peerValue) ) { \
            if( hb_filter_counter[#fieldName][peerValue]++ % FILTER_PRINT_INTERVAL == 0 ) { \
                DBHALOG_WARN("%s doesn't match. local(%s), peer(%s)",  #fieldName, localValue.c_str(), peerValue.c_str()); \
            } \
            return false; \
        } \
    }

#define COMPARE_INT_FIELD(fieldName, localValue, peerValue) { \
        if ( (localValue > 0 && peerValue > 0) && (localValue != peerValue) ) { \
            std::ostringstream oss; oss << peerValue; \
            std::string peerValueStr = oss.str(); \
            if( hb_filter_counter[#fieldName][peerValueStr]++ % FILTER_PRINT_INTERVAL == 0 ) { \
                DBHALOG_WARN("%s doesn't match. local(%d), peer(%d)",  #fieldName, localValue, peerValue); \
            } \
            return false; \
        } \
    }


// 判断消息是否属于同一个Heartbeat组
bool CHeartbeatMonitor::Internal::filter( TMulticastHeatBeatMsgPtr msg )
{
    if( m_get_hb_args_proc.empty() ) {
        DBHALOG_ERROR( "get heartbeat args callback proc is empty" );
        return false;
    }

    HeartbeatArgs args = m_get_hb_args_proc();

    COMPARE_STRING_FIELD( uid, args.uid, msg->getUid() );
    COMPARE_INT_FIELD( cluster_id, args.cluster_id, msg->getClusterId() );
    COMPARE_STRING_FIELD( service_name, args.service_name, msg->getServiceName() );
    COMPARE_STRING_FIELD( vip, args.vip, msg->getVirtualIp() );
    COMPARE_STRING_FIELD( user_defined, args.user_defined, msg->getUserDefined() );

    return true;
}

// 比较两个消息是否来自同一个Slave,true表示相等
bool CHeartbeatMonitor::Internal::isEqual( TMulticastHeatBeatMsgPtr lhs, TMulticastHeatBeatMsgPtr rhs )
{
    if ( lhs->getPktCmd() == rhs->getPktCmd() &&
         lhs->getPktType() == rhs->getPktType() &&
         lhs->getServiceName() == rhs->getServiceName() &&
         lhs->getLocalIp() == rhs->getLocalIp() ) {
        return true;
    } else {
        return false;
    }
}

TMulticastHeatBeatMsgPtr CHeartbeatMonitor::Internal::getMasterRequest()
{
    bool isRequest = true;
    return getMessage( isRequest );
}
// 获取作为slave，接收到master消息的回复包(上层不需要关心消息序号,由心跳模块内部设置消息序列号)
TMulticastHeatBeatMsgPtr CHeartbeatMonitor::Internal::getSlaveResponse()
{
    bool isRequest = false;
    return getMessage( isRequest );
}

TMulticastHeatBeatMsgPtr CHeartbeatMonitor::Internal::getMessage( bool isRequest )
{
    if( m_get_hb_args_proc.empty() ) {
        return NULL;
    }

    HeartbeatArgs args = m_get_hb_args_proc();

    TMulticastHeatBeatMsgPtr msg = TMulticastHeatBeatMsgPtr( new CMulticastHeartBeatMessage( isRequest ) );
    msg->setUid( args.uid );
    msg->setClusterId( args.cluster_id );
    msg->setVirtualIp( args.vip );
    msg->setLocalIp( args.local_ip );
    msg->setPeerIp( args.peer_ip );
    msg->setServiceName( args.service_name );
    msg->setUsedState( args.used_state );
    msg->setCurrentState( m_state );
    msg->setMasterTimeStamp( args.master_timestamp );
    msg->setUserDefined( args.user_defined );
	msg->setMasterBinlogName(args.master_log_file_name);
	msg->setMasterBinlogPos(args.master_log_file_position);
	//msg->setSlaveSqlState(args.slave_running_state);
	//msg->setSlaveSyncState(args.slave_sync_state);
	//msg->setMysqlMode(args.mysql_mode);
	msg->setMasterInitBinlogName(args.master_init_log_file_name);
	msg->setMasterInitBinlogPos(args.master_init_log_file_position);
	msg->setMasterChangeFlag(args.master_change_flag);
	msg->setMasterCurrentBinlog(args.master_current_binlog_name, args.master_current_binlog_pos, args.master_current_binlog_time);

//	DBHALOG_DEBUG("send hb message", );
    //msg->printPacket();
    return msg;
}

void CHeartbeatMonitor::Internal::pushState(bool errorFlag)
{
    LinkState links;
    bool curIsAllOffline = true;	//当前是否全部不在线
    bool preIsAllOffline = true;	//前面状态是否全部不在线
    bool isChanged = getLinkState( links, curIsAllOffline, preIsAllOffline );

	// 如果错误，则直接唤醒外层回调
	if (errorFlag){
		if (m_state == master){
			DBHALOG_ERROR("push state is error while state is master.");
		}else{
			DBHALOG_INFO("push error state into callback.");
		}
		m_callback->pushState(errordeal,links);
	}
    if( m_preAction == NoneAction ) { //第一次回调
        //第一次先回调主备机状态
        if( m_state == master ) {
            m_callback->pushState( becomeMaster, links );
            if( curIsAllOffline ) {
                //m_callback->pushState( slaveOffline, links );
                m_preAction = slaveOffline;
            } else {
                //m_callback->pushState( slaveOnline, links );
                m_preAction = slaveOnline;
            }
        } else if( m_state == slave ) {
            m_callback->pushState( becomeSlave, links );
            //m_callback->pushState( masterLinkChange, links );
            m_preAction = masterLinkChange;
        }
    } else if( isChanged ) {
        //其它时候只有改变了状态才回调
        if( m_state == master ) {
            HAAction action = slaveLinkChange;
            if( !preIsAllOffline && curIsAllOffline ) {
                // 从11,10,01到00
                action = slaveOffline;
            } else if( preIsAllOffline && !curIsAllOffline ) {
                // 从00到11,10,01
                action = slaveOnline;
				// 显式调用虚ip生效命令
				enableVip();
            }
            m_callback->pushState( action, links );
            m_preAction = action;
        } else if( m_state == slave ) {
            m_callback->pushState( masterLinkChange, links );
            m_preAction = masterLinkChange;
        }
    }
}

void CHeartbeatMonitor::Internal::pushStateAndWait( HAAction action )
{
	// 该函数设计时仅用于等待上层完成成主或成备的切换动作，因此不处理其他HAAction
	if( becomeMaster != action && becomeSlave != action ){
		return;
	}
	
	LinkState links;
    bool curIsAllOffline = true;
    bool preIsAllOffline = true;	
    (void)getLinkState( links, curIsAllOffline, preIsAllOffline );
	
	m_callback->pushStateAndWait( action, links );
	// 为了适配pushState()里的处理，必须修改m_preAction,避免pushState()中再次回上层进行主备切换
	if( becomeMaster == action ){
		m_preAction = curIsAllOffline ? slaveOffline : slaveOnline;
	}else{
		m_preAction = masterLinkChange;
	}
	
	return;
}

void CHeartbeatMonitor::Internal::addSendTimes( int32_t times )
{
    std::map<std::string, LinkInfo>::iterator it = m_interfaces.begin();
    for( ; it != m_interfaces.end(); it++ ) {
        LinkInfo &link_info = it->second;
        assert( !link_info.name.empty() );
        link_info.times += times;
    }
}

void CHeartbeatMonitor::Internal::resetLinkTimes( std::string source_ip )
{
    std::map<std::string, LinkInfo>::iterator it = m_interfaces.begin();
    for( ; it != m_interfaces.end(); ++it ) {
        // 比较发送端IP和本机IP的地址的前几位是否相同  todo: 有没有更好的方法获取本机链路IP
        if( 0 == source_ip.compare( 0, 3, it->first, 0, 3 ) ) {
            LinkInfo &link_info = it->second;
            assert( !link_info.name.empty() );
            if( link_info.times >= m_times ) {
                link_info.curState = online;
                DBHALOG_INFO( "link(%s) is connected",  link_info.name.c_str() );
            }
            link_info.times = 0;
            break;
        }
    }
    if ( m_interfaces.end() == it ) {
        DBHALOG_ERROR( "can't find message input link, src %s",  source_ip.c_str() );
    }
}

bool CHeartbeatMonitor::Internal::getLinkState( LinkState& link, bool& curIsAllOffline, bool& preIsAllOffline )
{
    bool isChanged = false;			//状态是否有改变

    std::map<std::string, LinkInfo>::iterator it = m_interfaces.begin();
    for( ; it != m_interfaces.end(); ++it ) {
        LinkInfo &link_info = it->second;
        assert( !link_info.name.empty() );

        if( link_info.times >= m_times ) {
            //超过设定次数改为超时状态
            link_info.curState = offline;
        }

        if( link_info.curState == online ) {
            //如果当前状态有在线的，则当前状态不是00
            curIsAllOffline = false;
        }
        if( link_info.preState == online ) {
            //如果前面状态有在线的，则前面状态不是00
            preIsAllOffline = false;
        }

        if( link_info.preState != link_info.curState ) {
            //状态有改变
            isChanged = true;
            link_info.preState = link_info.curState;
        }

        //返回当前状态
        link[link_info.name] = link_info.curState;
    }

    return isChanged;
}

bool CHeartbeatMonitor::Internal::checkLink(std::string ifName)
{
#ifdef DOCKER_MODE
	//小型化环境，cat /sys/class/net/eth1/operstate返回的是unknown，暂时注释掉
	return true;
#endif
	std::string strCmd = "cat /sys/class/net/" + ifName + "/operstate";
	std::ostringstream result;

	if( (Dahua::EFS::os::shell( &result, strCmd ) < 0)) {
		DBHALOG_ERROR( "shell failed."  );
		return false;
	}

	//DBHALOG_INFO( "shell return %s.",  result.str().c_str() );
	return result.str().find( "up" ) == std::string::npos ? false : true ;
}

bool CHeartbeatMonitor::Internal::checkBusinessLink()
{
	return checkLink(m_real_if_name);
}

bool CHeartbeatMonitor::Internal::checkHeartbeatLink()
{
	return checkLink(m_heartbeat_if_name);
}

void CHeartbeatMonitor::Internal::threadProc()
{
    while( looping() && !m_exit_flag ) {
        m_sem.pend();
        if( m_exit_flag ) {
            continue;
        }

        if( !m_stop_flag ){
			arping();
			enableVip();
        }
    }
}

CHeartbeatMonitor::CHeartbeatMonitor()
{
    m_internal = new struct Internal;
    DBHALOG_INFO( "m_internal is %p", m_internal );
}

CHeartbeatMonitor::~CHeartbeatMonitor()
{
    if( m_internal != NULL ) {
        delete m_internal, m_internal = NULL;
    } else {
        DBHALOG_INFO( "m_internal is NULL" );
    }
}

void CHeartbeatMonitor::setJudger( JudgerType type, Judger judger )
{
	if( m_internal != NULL ) {
        return m_internal->setJudger( type, judger );
    } else {
        DBHALOG_INFO( "m_internal is NULL" );
        return;
    }
}


void CHeartbeatMonitor::rmJudger( JudgerType type, Judger judger )
{
	if( m_internal != NULL ) {
        return m_internal->rmJudger( type, judger );
    } else {
        DBHALOG_INFO( "m_internal is NULL" );
        return;
    }
}

bool CHeartbeatMonitor::setMulticastAddress( const std::string& ip, int32_t port )
{
    if( m_internal != NULL ) {
        return m_internal->setMulticastAddress( ip, port );
    } else {
        DBHALOG_INFO( "m_internal is NULL" );
        return false;
    }
}

bool CHeartbeatMonitor::addInterface( const std::string& localIp )
{
    if( m_internal != NULL ) {
        return m_internal->addInterface( localIp );
    } else {
        DBHALOG_INFO( "m_internal is NULL" );
        return false;
    }
}

bool CHeartbeatMonitor::setVirtualIP( const std::string& vif_name, const std::string& if_name, 
									 const std::string& vip, const std::string& netmask, const bool& flag )
{
    if( m_internal != NULL ) {
        return m_internal->setVirtualIP( vif_name, if_name, vip, netmask, flag );
    } else {
        DBHALOG_INFO( "m_internal is NULL" );
        return false;
    }
}

bool CHeartbeatMonitor::setTimerPeriod( int32_t interval, int32_t times )
{
    if( m_internal != NULL ) {
        return m_internal->setTimerPeriod( interval, times );
    } else {
        DBHALOG_INFO( "m_internal is NULL" );
        return false;
    }
}

void CHeartbeatMonitor::setWaitInterval( uint32_t interval )
{
    if( m_internal != NULL ) {
        m_internal->setWaitInterval( interval );
    } else {
        DBHALOG_INFO( "m_internal is NULL" );
    }
}

void CHeartbeatMonitor::registerMsgCallback( MsgHandlerProc proc )
{
    if( m_internal != NULL ) {
        m_internal->registerMsgCallback( proc );
    } else {
        DBHALOG_INFO( "m_internal is NULL" );
    }
}

void CHeartbeatMonitor::unregisterMsgCallback()
{
    if( m_internal != NULL ) {
        m_internal->unregisterMsgCallback();
    } else {
        DBHALOG_INFO( "m_internal is NULL" );
    }
}

bool CHeartbeatMonitor::start( GetHbArgsCallback args_proc, HAStateProc state_proc )
{
    if( m_internal != NULL ) {
        return m_internal->start( args_proc, state_proc );
    } else {
        DBHALOG_INFO( "m_internal is NULL" );
        return false;
    }
}

bool CHeartbeatMonitor::stop()
{
    if( m_internal != NULL ) {
        return m_internal->stop();
    } else {
        DBHALOG_INFO( "m_internal is NULL" );
        return false;
    }
}

} // DBHA
} // VideoCloud
} // Dahua

