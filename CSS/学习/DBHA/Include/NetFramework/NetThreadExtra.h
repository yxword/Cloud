//
//  "$Id: NetThreadExtra.h 281999 2015-07-14 06:14:47Z shu_wang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-11		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_NET_THREAD_EXTRA_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_NET_THREAD_EXTRA_H__
#include "NDefs.h"

/*------------------郑重申明-----------------------*/
/*如下接口若需使用，请务必先与网络框架库负责人讨论，
经过充分论证以后方可使用。如无必要,不建议使用!!!*/


#ifdef __cplusplus
extern "C" {
#endif

namespace Dahua{
namespace NetFramework{

	//设置流媒体接收线程优先级。该函数在CreateThreadPool接口前调用。
	//参数：
	//		priority 	线程优先级，值越小表示优先级越高，会被转化成对应操作系统
	//	     		  	平台的优先级。取值1-127。
	// 	policy   	线程调度策略: 0, 普通线程; 1, 实时线程
	// 	stackSize 	为线程指定的堆栈大小，如果等于0或者小于平台要求必须的值，
	//        			则使用平台缺省值。
	//返回值：-1失败 0成功
int NETFRAMEWORK_API SetRecvThreadPriority( int priority, int policy = 0, int stack_Size = 0 );

}
}

#ifdef __cplusplus
}
#endif

#endif //__INCLUDED_DAHUA_NETFRAMEWORK_NET_THREAD_H__
