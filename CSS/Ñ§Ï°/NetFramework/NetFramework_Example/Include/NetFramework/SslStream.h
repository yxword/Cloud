//
//  "$Id: SslStream.h 30412 2011-07-21 05:57:44Z qin_fenglin $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-27		zmw         	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SSL_STREAM_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SSL_STREAM_H__

#include "Infra/IntTypes.h"
#include "SockAddr.h"
#include "SockStream.h"
#include "NDefs.h"
#ifdef WIN32
#include "NTypes_Win32.h"
#else
#include <sys/uio.h>
#endif

namespace Dahua{
namespace NetFramework{

class NETFRAMEWORK_API CSslStream : public CSock
{
	CSslStream& operator=( CSslStream const & other );
	CSslStream( CSslStream const & other );
public:
	CSslStream();
	~CSslStream();
	
	//请求ssl链接远端服务器
	//参数：remote 远端服务端的地址，包括IP和端口
	//		local 本地地址，包括IP地址和端口
	//返回值：-1 失败  0 成功
	int Connect(const CSockAddr & remote, CSockAddr* local = NULL );
	//在已经TCP握手情况下进行ssl握手
	//参数：sock 已经建立TCP连接的CSockStream指针
	//返回值：-1失败 0 成功
	int Connect( CSockStream* sock );  	
	//发送数据
	//参数：buf 待发送缓冲区 len 待发送数据的字节数
	//返回值：-1 失败 0 未发送出去 大于0的值表示已发送的长度
	int Send( const char * buf, uint32_t len );
	//接收数据
	//参数：buf 接收数据的缓冲 len 接收数据缓冲的大小
	//返回值：-1 失败 0 未接收数据 大于0 已接收数据字节数
	int Recv( char * buf, uint32_t len );
	//发送TCP数据，
	//参数；同Linux系统调用writev的参数。
	//返回值：-1失败，0未发送下次再试，大于0表示发送出去的长度。 
	int WriteV( const struct iovec *vector, int count );
	//带有超时的阻塞发送，接收，同样需要设置套接字为非阻塞模式。
	//超时时间单位为微秒，缺省为1秒。
	//参数：buf 待发送缓冲区 len 待发送数据的字节数 timeout 超时时间，默认为1秒
	//返回值：-1 失败 0  未发送数据 大于0 超时时间内发送的数据量
	int Send_n( const char * buf, uint32_t len, int timeout = 1000000 );
	//带超时的接收
	//参数：buf 接收缓冲区 len 接收缓冲区长度 timeout 超时时间，默认1秒
	//返回值：-1 失败 0 未接收到数据 大于0 实际接收到的数据量
	int Recv_n( char * buf, uint32_t len, int timeout = 1000000 );
	//设置ssl密钥文件cacert.pem和privkey.pem的路径，需要设置为系统路径
	//参数：path 系统密钥文件cacert.pem和privkey.pem的绝对路径
	//返回值：-1 失败 0 成功
	int SetPemPath( const char* path );
private:
	struct Internal;	
	struct Internal*	m_internal;		
};

}//namespace NetFramework
}//namespace Dahua

#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SSL_STREAM_H__
