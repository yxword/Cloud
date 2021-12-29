//
//  "$Id: NetThread.h 26645 2011-05-31 00:35:01Z wang_haifeng $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-11		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_NET_THREAD_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_NET_THREAD_H__

#include "Infra/IntTypes.h"
#include "NDefs.h"






namespace Dahua{
namespace NetFramework{

//这是整个框架模型的中心调度系统
class CNetThread
{
public:
	enum Priority
	{
		priorTop = 1,
		priorBottom = 127,
		priorDefault = 64,
	};
	enum Policy
	{
		policyNormal = 0,  ///< 普通线程
		policyRealtime = 1 ///< 实时线程
	};
	enum{
		HIGH_REALTIME = 0,
		REALTIME = 1,
		BALANCE = 2,
		PERFORMANCE = 3
	};
public:
	//创建一定数量的线程。该函数在系统初始化时调用
	//参数：thread_num为创建网络框架运行的线程数量。
	//		use_aio为是否使用异步磁盘IO，若置true，则需要使用CNFile来进行磁盘异步IO操作
	//		否则CNFile仅支持同步操作
	//返回值：-1失败 0成功
	static int NETFRAMEWORK_API CreateThreadPool( uint32_t thread_num, bool use_aio = false );
	//创建私有线程池
	//参数：thread_num为私有线程池中运行的线程数量。
	//		use_aio为是否使用异步磁盘IO，若置true，则需要使用CNFile来进行磁盘异步IO操作
	//		否则CNFile仅支持同步操作
	//返回值：-1，创建私有线程池失败
	//		>0, 创建私有线程池成功，返回值为私有线程池索引号，构造私有线程池中的NetHandler对象需要
	//		使用该索引值，以标识NetHandler对象属于哪个私有池
	static int NETFRAMEWORK_API CreatePrivatePool( uint32_t thread_num, bool use_aio = false );
	//设置网络框架在接收流媒体数据时的接收策略，注意：框架内部已经为流媒体接收设置了合理的策略，
	//若没有特殊需要，请不要随意调用该接口。该函数在CreateThreadPool接口前调用。
	//参数：level：指定的策略，可选的策略有：
	//						HIGH_REALTIME：高实时性。该策略下，尽可能快地接收网络上的数据，以牺牲性能为代价保证数据无滞留。
	//						REALTIME: 实时性，该策略下，保证绝大部分情况下，无延迟地接收网络上的流媒体数据。为默认策略。
	//						BALANCE：平衡，该策略兼顾性能与数据的实时性，在硬件性能良好，在实时性要求不高的情况下可选该策略。
	//						PERFORMANCE：性能优先，以牺牲实时性为代价保证性能。
	static void NETFRAMEWORK_API SetRcvPriority( int level );
	//设置网络框架线程优先级。该函数在CreateThreadPool接口前调用。
	//参数：
	//		priority 	线程优先级，值越小表示优先级越高，会被转化成对应操作系统
	//	     		  	平台的优先级。取值priorTop到priorBottom，默认值priorDefault。
	/// 	policy   	线程调度策略
	/// 	stackSize 	为线程指定的堆栈大小，如果等于0或者小于平台要求必须的值，
	///        			则使用平台缺省值。
	//返回值：-1失败 0成功
	static int NETFRAMEWORK_API SetThreadPriority( int priority, int policy = policyNormal, int stackSize = 0 );

	//销毁公有线程池。
	//返回值：0成功 无失败情况。
	static int NETFRAMEWORK_API DestroyThreadPool();
	//销毁索引为idx的私有线程池。
	//参数：idx私有线程池索引，为创建网络私有线程池时的返回值。
	//返回值：0成功 无失败情况。
	static int NETFRAMEWORK_API DestroyPrivatePool( int32_t idx );
	//遍历并打印注册到NEtFramework中的所有socket的fd,所属CNetHandler对象指针及其类型信息。
	//参数：无
	//返回值：注册NEtFramework中的所有socket的数量。
	static int NETFRAMEWORK_API TraversalSock();
	//开启R3 服务， 在创建线程池之后/ 销毁线程池之前调用
	//参数：无
	//返回值：0，开启成功，-1，开启失败
	static int NETFRAMEWORK_API EnableR3();
	//开启R3 服务，在创建线程池之后/ 销毁线程池之前调用
	//参数：无
	//返回值：0，关闭成功，无失败情况
	static int NETFRAMEWORK_API DisableR3();
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_NET_THREAD_H__
