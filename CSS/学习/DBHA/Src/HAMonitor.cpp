#include <math.h>
#include "HAMonitor.h"
#include "HALog.h"
#include "Infra/File.h"
#include "Infra/Time.h"
#include "Common/Timer.h"
#include "HAConfigs.h"
#include "Utils/Md5.h"
#include "Infra/Guard.h"
#include "CommonFun.h"

extern Dahua::VideoCloud::DBHA::MasterBinlogInfo gHeartBeatBinlogInfo;

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

using namespace EFS;
using namespace Infra;
using namespace std;
using namespace Utils;

DBHALOG_CLASSNAME( CHAMonitor );

enum 
{
    LF = ( int )( '\n' ),
    CR = ( int )( '\r' ),
    HT = ( int )( '\t' )
};

CHAMonitor::CHAMonitor( void )
{
    m_heartbeat_monitor = NULL;
    m_timer_id = -1;
}

CHAMonitor::~CHAMonitor( void )
{
    if( m_heartbeat_monitor ) {
        m_heartbeat_monitor->stop();
        delete m_heartbeat_monitor;
        m_heartbeat_monitor = NULL;
    }

    if( m_timer_id != -1 ){
		EFS::CTimer::instance()->detach(m_timer_id, true);
		m_timer_id = -1;
	}
}

void CHAMonitor::timeSlotProc( int64_t id )
{
	if( id != m_timer_id )
		return;

	setHeartBeatArgs();
}

bool CHAMonitor::mysqlGetCurrentBinlog(std::string &binlogName, int32_t &binlogPos)
{
	string initBinlogInfo = "";
	bool ret = CommonFun::getExecuteScriptValue(\
		CDBHAConfig::instance()->getBinlogInfoScript(), \
		"",initBinlogInfo);
	if (!ret){
		DBHALOG_ERROR("get binlog failed");
		return false;
	}

	return CommonFun::setBinlogInfo(initBinlogInfo, binlogName, binlogPos);
}

void CHAMonitor::setHeartBeatArgs()
{
	CHeartbeatMonitor::HeartbeatArgs args;
	args.uid = gHAConfig.getUId();
	args.cluster_id = gHAConfig.getClusterId();
	args.vip = gHAConfig.getVIP();
	args.local_ip = gHAConfig.getLocalBusinessIP();
	args.peer_ip = gHAConfig.getPeerBusinessIP();
	args.used_state = gHAConfig.getUsedState();
	args.master_timestamp = gHAConfig.getMasterTimestamp();

	//获取主机binlog相关配置信息
	gHAConfig.getInitBinlogInfo(args.master_init_log_file_name,
			args.master_init_log_file_position);

	gHAConfig.getChangeMasterInfo(args.master_change_flag, args.master_log_file_name,
			args.master_log_file_position);

	if ( mysqlGetCurrentBinlog(args.master_current_binlog_name, args.master_current_binlog_pos) ){
		args.master_current_binlog_time = Infra::CTime::getCurrentMilliSecond();
	}

	Infra::CGuardWriting writing(m_hb_mutex);
	m_hb_args = args;
}


bool CHAMonitor::startup( CHeartbeatMonitor::HAStateProc3 proc, JudgerMap judgers )
{
    DBHALOG_INFO( "HAMonitor entry startup." );

    /* 初始化心跳参数,避免组播组加入后冗余报文无法过滤 */
    setHeartBeatArgs();

    //启动定时器
	if( m_timer_id == -1 ){
		EFS::CTimer::TimeoutProc p( &CHAMonitor::timeSlotProc, this );
		m_timer_id = EFS::CTimer::instance()->attach( p );
	}

    //检查是否已经开启过.
    if( m_heartbeat_monitor != NULL ) {
        DBHALOG_INFO( "HAMonitor is started, need stop it first." );
        return false;
    }
    //回调函数声明.
    CHeartbeatMonitor::GetHbArgsCallback argsProc( &CHAMonitor::getHeartbeatArgs, this );
    CHeartbeatMonitor::HAStateProc stateProc( &CHAMonitor::handleHAStateChange, this );

    //分配内存.
    m_heartbeat_monitor = new CHeartbeatMonitor();
    if( !m_heartbeat_monitor ) {
        DBHALOG_ERROR( "alloc heartbeat monitor failed." );
        return false;
    }

    m_ha_state_proc = proc;

    do {
		// 因为在addInterface()后，就可能收到对端的消息开始选举，所以必须先添加判断规则
		for( JudgerMap::iterator it = judgers.begin(); it != judgers.end(); ++it ){
			std::list<CHeartbeatMonitor::Judger>::iterator judger_it = it->second.begin();
			for( ; judger_it != it->second.end(); ++judger_it ){
				m_heartbeat_monitor->setJudger( it->first, *judger_it );
			}
		}
		
        //设置组播地址.
        if( !m_heartbeat_monitor->setMulticastAddress( gHAConfig.getMulticastIP(), gHAConfig.getMulticastPort() ) ) {
            DBHALOG_ERROR( "set multicast[%s:%d] address failed.", gHAConfig.getMulticastIP().c_str(), gHAConfig.getMulticastPort() );
            break;
        }

		bool single_mode = false;
		if ((gHAConfig.getPeerBusinessIP() == "") || (gHAConfig.getLocalHeartbeatIP() == gHAConfig.getPeerBusinessIP()))
			single_mode = true;

        //设置虚IP
        if( !m_heartbeat_monitor->setVirtualIP( gHAConfig.getVIFName(), gHAConfig.getIFName(), gHAConfig.getVIP(), gHAConfig.getVIPNetmask(), single_mode ) ) {
            DBHALOG_ERROR( "set vip[%s,%s,%s,%s, %d] failed.", gHAConfig.getVIFName().c_str(),
                         gHAConfig.getIFName().c_str(), gHAConfig.getVIP().c_str(), gHAConfig.getVIPNetmask().c_str(), single_mode );
            break;
        }

		// 心跳不在，不用关心组播
		if (gHAConfig.getLocalHeartbeatIP() != ""){
			//增加心跳ip.
			 if( !m_heartbeat_monitor->addInterface( gHAConfig.getLocalHeartbeatIP() ) ) {
				 DBHALOG_ERROR( "add interface[%s] failed.", gHAConfig.getLocalHeartbeatIP().c_str() );
				 break;
			 }

			//改为判断业务IP和心跳IP是否相同如果相同则不需要加入组播地址
			//由于现在mysql主备虚IP改为心跳网卡，业务IP第二链路取消
			//但是该接口保留着防止后续继续沿用业务IP作为第二链路以及mysql虚IP绑在业务IP实网卡
			if (gHAConfig.getLocalHeartbeatIP() != gHAConfig.getLocalBusinessIP() )
			{
				//增加业务ip.
				if( !m_heartbeat_monitor->addInterface( gHAConfig.getLocalBusinessIP() ) ) {
					DBHALOG_ERROR( "add interface[%s] failed.", gHAConfig.getLocalBusinessIP().c_str() );
					break;
				}
			}
		}
        //设置定时器间隔和超时时间.
        if( !m_heartbeat_monitor->setTimerPeriod( gHAConfig.getMulticastTimeInterval( ),
                gHAConfig.getMasterDownInterval() / gHAConfig.getMulticastTimeInterval() ) ) {
            DBHALOG_ERROR( "set timer period failed." );
            break;
        }
        m_heartbeat_monitor->setWaitInterval( gHAConfig.getMasterWaitSlaveTime() );

        //启动心跳模块.
        if( !m_heartbeat_monitor->start( argsProc, stateProc ) ) {
            DBHALOG_ERROR( "heartbeat start failed." );
            break;
        }

        //如果集群id没有设置过,则设置一下获取主机的集群id.
        if( gHAConfig.getUId().empty() || gHAConfig.getClusterId() == 0 ) {
            DBHALOG_INFO( "set message proc." );
            CHeartbeatMonitor::MsgHandlerProc msgProc( &CHAMonitor::handleHeartbeatMsg, this );
            m_heartbeat_monitor->registerMsgCallback( msgProc );
        }

        return true;
    } while ( 0 );

    delete m_heartbeat_monitor;
    m_heartbeat_monitor = NULL;
    return false;
}

bool CHAMonitor::cleanup()
{
	DBHALOG_INFO( "ha monitor clean up begin." );
	if( m_timer_id != -1 ){
		EFS::CTimer::instance()->detach(m_timer_id, true);
		m_timer_id = -1;
	}

    if( m_heartbeat_monitor == NULL ) {
        DBHALOG_INFO( "hamonitor is not started,stop failed." );
        return false;
    }

    bool ret = m_heartbeat_monitor->stop();
    delete m_heartbeat_monitor;
    m_heartbeat_monitor = NULL;
    if( !ret ) {
        DBHALOG_ERROR( "hamonitor stop failed." );
    }

    DBHALOG_INFO( "ha monitor clean up end." );
    return ret;
}

CHeartbeatMonitor::HeartbeatArgs CHAMonitor::getHeartbeatArgs()
{
	Infra::CGuardReading reading(m_hb_mutex);
    return m_hb_args;
}

bool CHAMonitor::handleHAStateChange( CHeartbeatMonitor::HAAction action, CHeartbeatMonitor::LinkState& linkState )
{
	//回调给上层
	if( m_ha_state_proc.empty() ) {
		DBHALOG_ERROR( "ha state is empty." );
		return false;
	}
	
	// 错误处理，直接回调到上层，退出程序
	if (action == CHeartbeatMonitor::errordeal){ 
		DBHALOG_ERROR( "ha state is error." );
		m_ha_state_proc( action, linkState, false);
		return true;
	}

	//再调用上层业务之前设置uid保存配置
	if( action == CHeartbeatMonitor::becomeMaster ) {
		if( gHAConfig.getUId().empty() || gHAConfig.getClusterId() == 0 ) { //主机主动生成id.
			gHAConfig.setClusterId( createClusterID() );
			gHAConfig.setUId( createUID() );
			if( gHAConfig.saveConfig() ) {
				DBHALOG_ERROR("saveconfig failed.");
			}
		}
	}


	// 1：备机被清理过数据的，需要直接恢复  2：备机是扩容加入主备的，需要进行恢复。3：备机上次没有执行完恢复
	bool recovery_flag=false;
	if (g_DBHAConfig->getSlaveIsRecoveringFlag()||gHAConfig.getUsedState() == CHeartbeatMonitor::empty ||
			( g_DBHAConfig->serviceInExpandMode() && (CHeartbeatMonitor::master != gHAConfig.getUsedState())) )
		recovery_flag = true;

	//失败不保存配置.
	if( !m_ha_state_proc( action, linkState, recovery_flag ) ){
		if(action==CHeartbeatMonitor::becomeMaster){ //同步中需要重启心跳模块.
			
			m_heartbeat_monitor->stop(); //必然返回true.
			CHeartbeatMonitor::GetHbArgsCallback argsProc( &CHAMonitor::getHeartbeatArgs, this );
			CHeartbeatMonitor::HAStateProc stateProc( &CHAMonitor::handleHAStateChange, this );
			if( !m_heartbeat_monitor->start( argsProc, stateProc ) ) {
				DBHALOG_ERROR( "heartbeat start failed." );
			}
		}		

		return false;
	}

    //保存配置
    if( action == CHeartbeatMonitor::becomeMaster ){
		std::string binlog_name;
		int32_t binlog_pos;
		if ( mysqlGetCurrentBinlog(binlog_name, binlog_pos) ){
			gHAConfig.setChangeMasterInfo(binlog_name, binlog_pos);
		}else{
			//实时获取失败则使用500毫秒前心跳中保存的信息
			CHeartbeatMonitor::HeartbeatArgs args = getHeartbeatArgs();
			gHAConfig.setChangeMasterInfo(args.master_current_binlog_name, args.master_current_binlog_pos);
		}
		gHAConfig.saveHAState( CHeartbeatMonitor::master , Infra::CTime::getCurrentTime().makeTime() );
	}else if( action == CHeartbeatMonitor::becomeSlave ){
		//清理master binlog信息
		gHAConfig.setChangeMasterInfo("", 0);
		gHAConfig.saveHAState( CHeartbeatMonitor::slave , Infra::CTime::getCurrentTime().makeTime() );
	}

	return true;
}

void CHAMonitor::handleHeartbeatMsg( CHeartbeatMonitor::HAState localState,
                                     CHeartbeatMonitor::HAState remoteState,
                                     TMulticastHeatBeatMsgPtr msg )
{
    //在start的地方已经判断,只有集群id为空才设置回调.
    if( localState == CHeartbeatMonitor::slave ) { //集群id由主机生成.
		if( msg->getUid().empty() || msg->getClusterId() == 0 ){
			DBHALOG_INFO("cluster id is NULL.");
		}else{
			gHAConfig.setUId( msg->getUid() );
			gHAConfig.setClusterId( msg->getClusterId() );
			if( gHAConfig.saveConfig() ) {
				m_heartbeat_monitor->unregisterMsgCallback();
			}
		}
    } else if( localState == CHeartbeatMonitor::master ) {
		if( gHAConfig.getUId().empty() || gHAConfig.getClusterId() == 0 ) { //主机主动生成id.
			gHAConfig.setClusterId( createClusterID() );
			gHAConfig.setUId( createUID() );
			if( gHAConfig.saveConfig() ) {
				m_heartbeat_monitor->unregisterMsgCallback();
			}
		}
    }
}

string CHAMonitor::createUID()
{
    CGuard guard( m_mutex );

    uint64_t now = CTime::getCurrentMicroSecond();
    CMd5 md5;
    md5.init();
    char tmp[64] = {0};
    snprintf( tmp, sizeof( tmp ), "%" PRIu64 ":%p", now, &now );
    md5.update( ( const uint8_t* )tmp, sizeof( tmp ) );
    md5.hex( tmp );

    for( uint32_t i = 0; i < strlen( tmp ); i++ ) {
        tmp[i] = toupper( tmp[i] );
    }

    string uid = tmp;
    DBHALOG_INFO( "createUID: %s!\n", uid.c_str() );

    return uid;
}

uint8_t CHAMonitor::createClusterID()
{
    CGuard guard( m_mutex );

    uint8_t id;
    srand( time( NULL ) );
    do {
        id = rand() % ( int( pow( 2, 7 ) ) - 1 );
        if( id != LF && id != CR && id != HT && id != 0 ) {
            break;
        }
    } while( 1 );

    DBHALOG_INFO( "createClusterID: %d!\n", id );
    return id;
}


} // DBHA
} // VideoCloud
} // Dahua

