//
//  "$Id: SockStream.h 30412 2011-07-21 05:57:44Z qin_fenglin $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-10		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_STREAM_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_STREAM_H__

#ifdef WIN32
#include "NTypes_Win32.h"
#else
#include <sys/uio.h>
#endif
#include "SockAddr.h"
#include "Sock.h"
#include "NDefs.h"
#include <string.h>

namespace Dahua{
namespace NetFramework{

class NETFRAMEWORK_API CSockStream : public CSock
{
	CSockStream& operator=(CSockStream const& other);
	CSockStream(CSockStream const& other);
public:
	CSockStream();
	~CSockStream();
	
	//连接对端的TCP侦听地址
	//参数 remote 对端的TCP地址
	//		local 本地地址，绑定特定的IP地址和端口
	//返回值：-1 失败	0不表示成功，要在稍后的GetConnectStatus中判断是否成功。
	int Connect(const CSockAddr & remote,CSockAddr* local = NULL);
	//连接的状态：已连接；未连接成功；连接错误。
	enum CONNECT_STATUS{ 
		STATUS_CONNECTED = 0, 
		STATUS_NOTCONNECTED = -1, 
		STATUS_ERROR = -2 
	};
	//获取连接的状态，返回值为上述的三个值之一。
	int GetConnectStatus();

	//TCP的发送，接收缓冲设置，以及TCP发送，接收缓冲区大小的获取。
	//参数：意义同系统调用setsockopt中对应的参数。
	//返回值： Set: -1失败，0成功
	//				 Get: 实际获取到的缓冲区的大小。
	int SetSockRecvBuf( uint32_t len );
	int GetSockRecvBuf( );
	int SetSockSendBuf( uint32_t len );
	int GetSockSendBuf( );
	//保活设置。
	//参数： false不设置，true设置
	//返回值：-1失败，0成功
	int SetSockKeepAlive( bool keepalive );
	//设置发送无延迟。
	//参数：false不设置，true设置
	//返回值: -1失败 0成功
	int SetSockNoDelay( bool nodelay );
	//发送，接收TCP数据
	//参数：buf待发送数据或接收缓冲 len待发送数据长度或接收缓冲长度。
	//返回值：-1失败，0未发送下次再试，大于0表示发送出去或接收到的长度。
	//说明：Recv返回-1，且errno==ECONNRESET时，表示对端主动断开连接。
	int Send( const char * buf, uint32_t len );
	int Recv( char * buf, uint32_t len );
	//发送TCP数据，
	//参数；同Linux系统调用writev的参数。
	//返回值：-1失败，0未发送下次再试，大于0表示发送出去的长度。 
	int WriteV( const struct iovec *vector, int count );

	//带有超时的阻塞发送，接收，同样需要设置套接字为非阻塞模式。
	//超时时间单位为微秒，缺省为1秒。
	//返回值：同上。
	int Send_n( const char * buf, uint32_t len, int timeout = 1000000 );
	int Recv_n( char * buf, uint32_t len, int timeout = 1000000 );
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_STREAM_H__
