//
//  "$Id: NetFrameworkThreadsMonitor.h  2015年10月29日  zhang_hailong 23919 $"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day     SVN-Author         Modification
//					2015年10月29日	   zhang_hailong	  Create
//

#ifndef __INCLUDE_DAHUA_EFS_NETFRAMEWORKTHREADSMONITOR_H__
#define __INCLUDE_DAHUA_EFS_NETFRAMEWORKTHREADSMONITOR_H__

namespace Dahua {
namespace EFS {

// 用于监测网络框架线程是否全部都被阻塞。
// 使用网络框架内部定时器,每1秒响应一次.如两次响应之间的时间间隔超过2秒,打印告警日志,说明在此期间网络框架线程全部阻塞
// 参见问题单 #6745,解决DN离线问题时,参考SDK代码引入

// 使用方法: 调用网络框架线程池初始化函数后,定义一个 CNetFrameworkThreadsMonitor
// 网络框架线程退出前,调用 CNetFrameworkThreadsMonitor::close()
// eg:
//		NetFramework::CNetThread::CreateThreadPool( 128 );
//		CNetFrameworkThreadsMonitor monitor;
//		run();     // run some business
//		monitor.close();
class CNetFrameworkThreadsMonitor
{
public:
	CNetFrameworkThreadsMonitor();
	virtual ~CNetFrameworkThreadsMonitor();
	void    close();
private:
	class Internal;
	Internal*	m_internal;
};

} /* namespace EFS */
} /* namespace Dahua */
#endif /* __DAHUA_EFS_NETFRAMEWORKTHREADSMONITOR_H__ */
