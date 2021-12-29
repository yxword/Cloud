//
//  "$Id: SockStream.h 7672 2010-5-10 02:28:18Z he_linqiang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-10		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_DHTS_STREAM_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_DHTS_STREAM_H__

#include "NDefs.h"
#include "SockAddr.h"
#include "Sock.h"
#include <string.h>
#ifndef WIN32
#include <sys/uio.h>
#endif

namespace Dahua{
namespace NetFramework{

class NETFRAMEWORK_API CDhtsStream : public CSock
{
	CDhtsStream& operator=(CDhtsStream const& other);
	CDhtsStream(CDhtsStream const& other);
public:
	CDhtsStream();
	~CDhtsStream();
	
	//连接对端的TCP侦听地址
	//参数 remote 对端的TCP地址
	//		local 本地地址，多ip时绑定特定ip地址
	//返回值：-1 失败	0不表示成功，要在稍后的GetConnectStatus中判断是否成功。
	int Connect(const CSockAddr & remote,CSockAddr* local = NULL);
	//连接的状态：已连接；未连接成功；连接错误;
	enum CONNECT_STATUS{ 
		STATUS_CONNECTED = 0, 
		STATUS_NOTCONNECTED = -1, 
		STATUS_ERROR = -2 
	};
	//获取连接的状态，返回值为上述的三个值之一?
	int GetConnectStatus();

	//TCP的发送，接收缓冲设置，以及TCP发送，接收缓冲区大小的获取。
	//参数：意义同系统调用setsockopt中对应的参数。
	//返回值： Set: -1失败，0成功
	//				 Get: 实际获取到的缓冲区的大小?
	int SetSockRecvBuf( uint32_t len );
	int GetSockRecvBuf( );
	int SetSockSendBuf( uint32_t len );
	int GetSockSendBuf( );
	
	//DHTS流式数据发送、接收接口
	//参数 buf待发送、接收的数据缓冲区; len待发送数据长度或接收缓冲长度。
	//		flags 暂时无用
	//返回值：-1 失败，0未发送下次再试，大于0表示发送出去或接收到的长度
	int Send( const char * buf, uint32_t len, int flags=0);
	int Recv( char * buf, uint32_t len, int flags=0);

	//DHTS数据报式数据发送、接收接口
	//参数 buf待发送、接收的数据缓冲区; len待发送数据长度或接收缓冲长度。
	//		ttl_ms 半可靠模式下, 消息在发送队列的最大存活时间,ms单位
	// 		inorder 是否消息顺序递交到接收端(不常用)
	//返回值：-1 失败，0未发送下次再试，大于0表示发送出去或接收到的长度
	//注意：  当ttl = -1, 为正常的全可靠模式(默认模式);
	//		  当ttl > 0, 为半可靠模式
	int Sendmsg( const char * buf, uint32_t len, int ttl_ms = -1, bool inorder = true);
	int Recvmsg( char * buf, uint32_t len );
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_DHTS_STREAM_H__


