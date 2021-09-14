//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_SERVER_H__
#define __INCLUDE_DAHUA_EFS_SERVER_H__

#include "NetFramework/SockAddr.h"
#include "NetFramework/SockStream.h"

namespace Dahua {
namespace EFS {

class CServiceBase;
//服务基类
class CServer
{	
public:
	CServer();
	virtual ~CServer();

	/// 初始化服务
	/// \param local 本地网络地址，IP+Port
	/// \return 0成功，-1失败
	int init( NetFramework::CSockAddr& local );
	
	/// 创建异步执行线程池，调用此接口之后，Server端
	/// 网络接收和处理分离，单连接可以实现并发，同时
	/// 需要注意session内的请求会并发到达
	/// \param num 线程池线程个数
	void createAsynThreads( uint16_t num );
	
	/// 添加会话，将创建出来的会话和Server关联
	bool addSession( CServiceBase* ptr );
	
	/// 删除会话
	bool delSession( CServiceBase* ptr );
	
	/// 获得会话个数
	uint32_t getSessionNum();
	
	/// 关闭
	int close();

	/// TCP连接处理接口，一旦有TCP握手请求到达，此接口会被调用，子类必须实现
	/// \param sock 套接字描述符封装
	/// \return 0成功，-1失败
	virtual int accept( NetFramework::CSockStream& sock ) = 0;
private:
	class Internal;
	Internal*	m_internal;
};

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_SERVER_H__
