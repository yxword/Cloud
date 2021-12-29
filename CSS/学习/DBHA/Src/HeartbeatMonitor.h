//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __DAHUA_VIDEOCLOUD_DBHA_HEARTBEAT_MONITOR_H__
#define __DAHUA_VIDEOCLOUD_DBHA_HEARTBEAT_MONITOR_H__

#include "Infra/IntTypes.h"
#include "Infra/Mutex.h"
#include "Infra/Function.h"
#include "Infra/Thread.h"
#include "Infra/Guard.h"
#include "Infra/Semaphore.h"
#include "Infra/Time.h"
#include "Common/PacketBase.h"
#include "Common/ProtoParser.h"
#include "MulticastHeartBeatMessage.h"
#include <map>
#include <vector>
#include <string>
#include <list>

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

class CHeartbeatMonitor
{
public:

    typedef struct _HeartbeatArgs 
    {
        std::string uid;                // uid
        uint8_t cluster_id;             // cluster id
        std::string vip;                // 虚IP.
        std::string local_ip;           // 本机IP.
        std::string peer_ip;            // 对端IP.
        std::string service_name;       // 服务名称, 可以用来区分服务.
        std::string user_defined;       // 用户自定义.
        int32_t used_state;             // 本机历史状态.
        int32_t master_timestamp;       // 本机成为master的时间戳.
		//新增mysql数据库相关字段
		//HA成为主机后,需要把主机的binglog文件与位置发给备机
		std::string master_log_file_name; //主机binlog文件名
		int32_t master_log_file_position; //主机binlog文件位置
		//当主备关系组成以后备机需要将自己的同步状态和sql线程状态发给主机供运维查询
		//int32_t slave_running_state;	//mysql备机sql状态
		//int32_t slave_sync_state;		//mysql备机IO状态
		//int32_t mysql_mode;				//mysql主备状态

		std::string master_init_log_file_name; //主机初始binlog文件名
		int32_t master_init_log_file_position; //主机初始binlog文件位置

		int32_t master_change_flag;		//主机上次是否合法成为主
		int32_t master_current_binlog_pos;		//主机当前的binlog位置
		std::string master_current_binlog_name;	//主机当前的binlog名字
		uint64_t master_current_binlog_time;	//主机获取当前binlog的时间

        _HeartbeatArgs()
        {
            cluster_id = 0;
            service_name = "UnknownService";
            used_state = -1;
            master_timestamp = 0;
			master_log_file_position = 0;
			//slave_running_state = 1;
			//slave_sync_state = 1;
			//mysql_mode=0;
			master_init_log_file_position = 0;
			master_change_flag = 0;
			master_current_binlog_pos = 0;
			master_current_binlog_name = "";
			master_current_binlog_time = Infra::CTime::getCurrentMilliSecond();
			
        }
    } HeartbeatArgs;

    typedef enum 
    {
        init = -2,
        prepare = -1,
        slave = 0,
        master = 1,
        // empty由MysqlServer.sh直接修改配置文件生成
        // NOTE:目前已经不存在该状态了
		empty = 2, // 数据库被清空过
		dead = 3, // 当变为该状态时，意味着程序要退出了
    } HAState;

    typedef enum 
    {
        online = 0,
        offline
    } OnlineState;

    typedef enum 
    {
        NoneAction = -1,		//无动作，内部初始化使用.
        becomeMaster = 0,		//成为主机.
        becomeSlave = 1,		//成为备机.
        slaveOnline = 2,		//备机上线.
        slaveOffline = 3,		//备机下线.
        slaveLinkChange = 4,	//备机链路状态发生改变.
        masterLinkChange = 5,	//主机链路发送改变.
		errordeal = 6,			// 错误处理
    } HAAction;

	typedef enum
	{
		mysqlNoInit = 0,
		mysqlMaster = 1,
		mysqlSlave = 2

	}mysqlModeStatus;

	typedef enum
	{
		waitingForSync = -1,
		timeoutForSync = 0,
		totallySync = 1
	}checkSyncState;

	// 仲裁者类型
	enum JudgerType
	{
		BECOME_MASTER,  ///< 成为主
		FAST_BECOME_MASTER, ///< 快速成为主
	};
		
    // 链路状态.
    typedef std::map<std::string, OnlineState> LinkState;

    // 集群中其他节点上下线回调.
    typedef Infra::TFunction0<HeartbeatArgs> GetHbArgsCallback;

    // 集群中其他节点上下线回调: 参数1-链路IP,例如172.5.5.119  参数2-该链路当前状态.
    typedef Infra::TFunction2<bool, HAAction, LinkState&> HAStateProc;

	// 集群中其他节点上下线回调: 参数1-链路IP,例如172.5.5.119  参数2-该链路当前状态.
	typedef Infra::TFunction3<bool, HAAction, LinkState&, bool> HAStateProc3;

    // 收到消息后回调上层接口 (见问题单 #4247).
    // 各参数含义依次是: 本机HA状态; 远端HA状态; 本次收到的消息.
    typedef Infra::TFunction3<void, HAState, HAState, TMulticastHeatBeatMsgPtr> MsgHandlerProc;

	// 能否成主的判断函数,通过参数返回判断条件的文字描述
	typedef Infra::TFunction1<bool, std::stringstream&> Judger;
	
    CHeartbeatMonitor();
    ~CHeartbeatMonitor();

	/**
	 * @brief 设置仲裁规则
	 * @param type 规则类型
	 * @param proc 判断函数
	 */
	void setJudger( JudgerType type, Judger judger );

	/**
	 * @brief 移除仲裁规则
	 * @param type 规则类型
	 * @param proc 判断函数
	 */
	void rmJudger( JudgerType type, Judger judger );
	
    // 设置组播地址、端口.
    // 注意: 该接口和addInterface调用不分先后,但必须都调用至少一次.
    bool setMulticastAddress( const std::string& ip, int32_t port );

    // 增加组播出口网卡，多次调用可以实现多网卡路径心跳.
    // localIp: 本机地址,例如本机业务线IP 172.5.5.119 或 心跳线IP为 192.168.0.119.
    bool addInterface( const std::string& localIp );

    // 设置虚IP地址，成为master之后使能虚IP，并实现周期arp.
    // vif_name: 虚拟网卡名,例如 bond0:1, 用于执行  ifconfig bond0:1 172.5.4.10 netmask 255.255.0.0 up.
    // if_name : 真实网卡名,例如bond0, 用于执行  arping -b -A -U -c 1  -I bond0 172.5.4.10.
    // ip : 虚IP,例如  172.5.4.10.
    // netmask : 虚IP的掩码,例如 255.255.0.0.
    bool setVirtualIP( const std::string& vif_name, const std::string& if_name, 
		const std::string& vip, const std::string& netmask, const bool& flag );

    // 设置定时器超时时间，interval为master组播时间间隔，times表示多少个interval间隔slave.
    // 没有接收到包可以成为master.
    bool setTimerPeriod( int32_t interval, int32_t times );

    // 本机进入prepare并等待若干个心跳周期后,如果没有收到response,就直接进入master状态.
    void setWaitInterval( uint32_t interval );

    // 设置心跳消息回调函数.如该回调被设置，当收到消息后，会调用该函数将消息交给上层继续处理。 目前是为了解决 #4247 问题才添加该接口
    // 正常情况下,心跳消息比较频繁，如果每个消息都回调给上层,开销会很大。因此没有特殊需要的话不建议设置该回调,如设置该回调函数,在需求完成后建议及时取消注册。
    // 该回调和 start 接口中state_proc,online_proc 都在同一个线程中被调用.
    void registerMsgCallback( MsgHandlerProc proc );
    void unregisterMsgCallback();

    // 开启检测(调用start之前,必须先调用上面的set系列接口.set相关接口调用顺序部分先后)
    // args_proc,   获取本机HA参数的回调函数,该函数应该返回本机IP、对端IP、当前状态、历史状态、master时间戳等信息;
    // state_proc,  本机HA状态发生变化时的回调函数,用于通知CS/MDS 上层对HA状态变化作出处理;
    // 注意：args_proc 心跳主线程中频繁调用args_proc 以获取心跳相关参数, 而online_proc 和 state_proc 都是在一个线程里串行调用的
    //               上层应避免args_proc和其他两个回调函数存在锁竞争等情况.
    bool start( GetHbArgsCallback args_proc, HAStateProc state_proc );

    // 停在检测.目前没有实现.
    bool stop();

private:
    class Internal;
    Internal* m_internal;
};

} // DBHA
} // VideoCloud
} // Dahua

#endif // __DAHUA_VIDEOCLOUD_DBHA_HEARTBEAT_MONITOR_H__
