//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef WIN32

#ifndef __INCLUDE_HEART_BEAT_MONITOR_H__
#define __INCLUDE_HEART_BEAT_MONITOR_H__

#include "Infra/IntTypes.h"
#include "Infra/Function.h"
#include "Common/InnerVersion.h"
#include "Message/HeartBeatMessage.h"
#include <map>
#include <vector>
#include <string>
#include <list>

namespace Dahua {
namespace EFS {

class CHeartbeatMonitor
{
public:
	typedef enum
	{
		UnknownService = -1,
		MdsService = 0,
		CsService = 1,
		LsService = 2,
		ProxyService = 3
	} HASvrType;

	typedef struct _HeartbeatArgs
	{
		std::string mds_uid;        // mds uid, MDS 心跳使用
		uint8_t cluster_id;     // cluster id, MDS 心跳使用
		std::string mds_ip;         // mds IP, CS 心跳使用
		std::string vip;            // 虚IP, MDS CS 通用
		std::string local_ip;       // 本机IP, MDS CS 通用
		std::string peer_ip;        // 对端IP, MDS CS 通用
		std::string mac_addr;       // 本机mac, MDS 心跳保留字段
		uint8_t ip_count;       // MDS 心跳保留字段
		CInnerVersion version;        // MDS 心跳保留字段
		HASvrType service_type;       // 服务类型,区分是MDS还是CS
		int32_t used_state;         // 本机历史状态
		int32_t master_timestamp;   // 本机成为master的时间戳

		_HeartbeatArgs()
		{
			cluster_id = 0;
			ip_count = 0;
			service_type = UnknownService;
			used_state = -1;
			master_timestamp = 0;
		}
	} HeartbeatArgs;

	typedef enum
	{
		init = -2,
		prepare = -1,
		slave = 0,
		master = 1,
	} HAState;

	typedef enum
	{
		online = 0,
		offline
	} OnlineState;

	typedef enum
	{
		noneAction = -1,		//初始态,不会回调这个状态出来.无动作，内部初始化使用。
		becomeMaster = 0,		//成为主机
		becomeSlave = 1,		//成为备机
		slaveOnline = 2,		//备机上线
		slaveOffline = 3,		//备机下线
		slaveLinkChange = 4,	//备机链路状态发生改变
		masterLinkChange = 5,	//主机链路发送改变
	} HAAction;
	
	// 链路状态
	typedef std::map<std::string, OnlineState> LinkState;
	
	// 集群中其他节点上下线回调
	typedef Infra::TFunction0<HeartbeatArgs> GetHbArgsCallback;

	// 集群中其他节点上下线回调: 参数1-链路IP,例如172.5.5.119  参数2-该链路当前状态
	typedef Infra::TFunction2<void, HAAction, LinkState&> HAStateProc;

	// 收到消息后回调上层接口 (见问题单 #4247)
	// 各参数含义依次是: 本机HA状态; 远端HA状态; 本次收到的消息
	typedef Infra::TFunction3<void, HAState, HAState, THeatBeatMsgPtr> MsgHandlerProc;

	CHeartbeatMonitor();
	~CHeartbeatMonitor();

	// 设置组播地址、端口
	// 注意: 该接口和addInterface调用不分先后,但必须都调用至少一次
	bool setMulticastAddress( const std::string& ip, int32_t port );

	// 增加组播出口网卡，多次调用可以实现多网卡路径心跳
	// localIp: 本机地址,例如本机业务线IP 172.5.5.119 或 心跳线IP为 192.168.0.119
	bool addInterface( const std::string& localIp );

	// 设置虚IP地址，成为master之后使能虚IP，并实现周期arp
	// vif_name: 虚拟网卡名,例如 bond0:1, 用于执行  ifconfig bond0:1 172.5.4.10 netmask 255.255.0.0 up
	// if_name : 真实网卡名,例如bond0, 用于执行  arping -b -A -U -c 1  -I bond0 172.5.4.10
	// ip : 虚IP,例如  172.5.4.10
	// netmask : 虚IP的掩码,例如 255.255.0.0
	bool setVirtualIP( const std::string& vif_name, const std::string& if_name, const std::string& vip, const std::string& netmask );

	// 添加虚IP地址，成为master之后使能虚IP，并实现周期arp
	// vif_name: 虚拟网卡名,例如 bond0:1, 用于执行  ifconfig bond0:1 172.5.4.10 netmask 255.255.0.0 up
	// if_name : 真实网卡名,例如bond0, 用于执行  arping -b -A -U -c 1  -I bond0 172.5.4.10
	// ip : 虚IP,例如  172.5.4.10
	// netmask : 虚IP的掩码,例如 255.255.0.0
	// 新增接口支持一条心跳链路多个虚IP场景，保留setVirtualIP兼容老的代码
	bool addVirtualIP( const std::string& vif_name, const std::string& if_name, const std::string& vip, const std::string& netmask );

	// 设置定时器超时时间，interval为master组播时间间隔，times表示多少个interval间隔slave
	// 没有接收到包可以成为master
	bool setTimerPeriod( int32_t interval, int32_t times );

	// 本机进入prepare并等待若干个心跳周期后,如果没有收到response,就直接进入master状态
	void setWaitInterval( uint32_t interval );

	// 设置心跳消息回调函数.如该回调被设置，当收到消息后，会调用该函数将消息交给上层继续处理。 目前是为了解决 #4247 问题才添加该接口
	// 正常情况下,心跳消息比较频繁，如果每个消息都回调给上层,开销会很大。因此没有特殊需要的话不建议设置该回调,如设置该回调函数,在需求完成后建议及时取消注册。
	// 该回调和 start 接口中state_proc,online_proc 都在同一个线程中被调用。
	void registerMsgCallback(MsgHandlerProc proc);
	void unregisterMsgCallback();

	// 开启检测(调用start之前,必须先调用上面的set系列接口.set相关接口调用顺序部分先后)
	// args_proc,   获取本机HA参数的回调函数,该函数应该返回本机IP、对端IP、当前状态、历史状态、master时间戳等信息;
	// state_proc,  本机HA状态发生变化时的回调函数,用于通知CS/MDS 上层对HA状态变化作出处理;
	// 注意：args_proc 心跳主线程中频繁调用args_proc 以获取心跳相关参数, 而online_proc 和 state_proc 都是在一个线程里串行调用的
	//               上层应避免args_proc和其他两个回调函数存在锁竞争等情况
	bool start( GetHbArgsCallback args_proc, HAStateProc state_proc);

	// 停在检测.目前没有实现
	bool stop();

private:
	class Internal;
	Internal* m_internal;
};

}
}

#endif //__INCLUDE_HEART_BEAT_MONITOR_H__
#endif // WIN32

