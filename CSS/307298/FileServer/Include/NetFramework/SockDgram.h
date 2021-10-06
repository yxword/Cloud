//
//  "$Id: SockDgram.h 54643 2012-03-17 09:31:34Z zhou_mingwei $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-7		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_DGRAM_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_DGRAM_H__

#include "SockAddr.h"
#include "Sock.h"
#include "NDefs.h"
#include <string.h>
#ifdef WIN32
#include "NTypes_Win32.h"
#else
#include <sys/uio.h>
#endif

namespace Dahua{
namespace NetFramework{

//这是UDP套接字的定义
class NETFRAMEWORK_API CSockDgram: public CSock
{
	CSockDgram& operator=(CSockDgram const& other);
	CSockDgram(CSockDgram const& other);
public:
	CSockDgram();
	~CSockDgram();
	//发送UDP数据报
	//参数：buf 待发送的数据，len 待发送数据的长度  remote 发送对端的地址。
	//返回值：-1 失败 0 未发送出去，下次重试 大于0的值表示已发送的长度。
	int Send( const char * buf, uint32_t len, const CSockAddr * remote = NULL );
	//发送UDP数据报
	//参数；同Linux系统调用writev的参数。
	//返回值：-1失败，0未发送下次再试，大于0表示发送出去的长度。 
	int WriteV( const struct iovec *vector, int count );	
	//接收UDP数据报
	//参数：buf 接收数据的缓冲 len 接收数据缓冲的大小 remote 接收数据的对端地址。
	//返回值：-1 失败 0 未接收到，下次重试 大于0的值表示已接收到的长度。
	int Recv(char * buf, uint32_t len, CSockAddr* remote = NULL);
	
	//打开一个UDP套接字。
	//参数：addr 待绑定的本地地址，也可以是组播地址，表示加入该组播组。
	//返回值：-1 失败 0 成功。
	int Open(const CSockAddr * addr = NULL);
	//打开一个UDP套接字
	//参数：addr 待绑定的本地地址，也可以是组播地址，表示加入该组播组。
	//		reuseaddr 是否设置SO_REUSEADDR,一般用于服务器端口重用
	//返回值：-1 失败 0 成功。
	int Open(const CSockAddr *addr, bool reuseaddr );
	//设置远端ip和端口，设置之后在Send时可以不再设置远端地址
	//参数 :	remote接收数据的对端地址
	//返回值:-1失败 0成功
	int SetRemote( const CSockAddr* remote );
public:
	//以下接口都是多播相关的，传入参数均为主机字节序。
	//设置多播发送的本地接口
	//返回值：-1 失败 0 成功
	int SetMulticastIF( uint32_t ip_h );
	int SetMulticastIF( const char * ipstr );
	//设置多播发送报文的TTL
	//返回值： -1 失败 0 成功
	int SetMulticastTTL( uint8_t TTL );
	//设置是否禁止组播数据回送，true-回送，false-不回送。
	//返回值: -1 失败 0 成功
	int SetMulticastLoop( bool isloop );
private:
	struct Internal;	
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_DGRAM_H__
