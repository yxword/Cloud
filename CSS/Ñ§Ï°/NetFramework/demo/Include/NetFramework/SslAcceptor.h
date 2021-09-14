//
//  "$Id: SslAcceptor.h 30412 2011-07-21 05:57:44Z qin_fenglin $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-27		zmw        	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SSL_ACCEPTOR_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SSL_ACCEPTOR_H__

#include "NDefs.h"
#include "SslStream.h"
#include "SockAddr.h"
#include <string.h>

namespace Dahua{
namespace NetFramework{

class NETFRAMEWORK_API CSslAcceptor : public CSock 
{
	CSslAcceptor& operator=(CSslAcceptor const& other);
	CSslAcceptor(CSslAcceptor const& other);
public:
	CSslAcceptor();
	~CSslAcceptor();
public:
	//服务器端侦听本地端口
	//参数：local 本地地址信息，ip和端口
	//返回值：-1失败  0 成功
	int Open( const CSockAddr & local );
#ifndef WIN32
	//服务器接收链接请求
	//参数：remote 调用后记录连接请求的客户端地址信息
	//返回值：CSslStream类型指针，NULL 失败，否则成功
	CSslStream* Accept( CSockAddr * remote = NULL );
#endif
	//设置ssl密钥文件cacert.pem和privkey.pem的路径，需要设置为系统路径
	//参数：path 系统密钥文件cacert.pem和privkey.pem的绝对路径
	//返回值：-1 失败 0 成功
	int SetPemPath( const char* path );
	//接受一个远端的连接请求
	//参数：sock用于填充接受的连接请求
	//		remote远端地址
	//返回值：-1：接收失败，sock不可用 0：成功，sock可用
	int Accept( CSslStream& sock, CSockAddr * remote = NULL );
	//接受一个远端的SSL连接请求,不用调用Open
	//参数：sock 用于填充接受的连接请求
	// stream 已经建立TCP连接的CSockStream指针
	//返回值：-1：接收失败，sock不可用 0：成功，sock可用
	int Accept(CSslStream& sock, CSockStream* stream);
private:
	struct Internal;	
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua

#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SSL_ACCEPTOR_H__
