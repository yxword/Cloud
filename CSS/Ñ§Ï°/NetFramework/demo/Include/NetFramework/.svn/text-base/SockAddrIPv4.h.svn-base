//
//  "$Id: SockAddrIPv4.h 30412 2011-07-21 05:57:44Z qin_fenglin $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-5-7        he_linqiang Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_ADDRIPV4_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_ADDRIPV4_H__

#include "SockAddr.h"
#include "NDefs.h"
#ifdef WIN32
#ifdef SetPort
#undef SetPort
#endif  //endif SetPort
#include "NTypes_Win32.h"
#else
#include <netinet/in.h>
#endif  //endif WIN32

namespace Dahua{
namespace NetFramework{

class NETFRAMEWORK_API CSockAddrIPv4 : public CSockAddr
{
public:
	// 提供多种形式的初始化接口，传入参数均为主机字节序。
	CSockAddrIPv4();
	CSockAddrIPv4( uint32_t ip_h, uint16_t port_h );
	CSockAddrIPv4( const char * ipstr, uint16_t port_h );
	CSockAddrIPv4( const struct sockaddr_in & addr );
	CSockAddrIPv4( CSockAddrIPv4 const & other );
	~CSockAddrIPv4();
	bool operator==( CSockAddrIPv4 & other)const;
	CSockAddrIPv4& operator=( CSockAddrIPv4 const & other );
public:
	virtual bool IsValid() const;

	virtual struct sockaddr * GetAddr( struct sockaddr * addr ) const; 
	virtual uint32_t GetAddrLen() const;
public:
	//提供尽可能丰富的设置接口。设置成功返回值>=0，设置失败，返回-1。
	int SetAddr( uint32_t ip_h, uint16_t port_h );
	int SetAddr( const char * ipstr, uint16_t port_h );
	int SetAddr( const struct sockaddr_in & addr );
	int SetIp( uint32_t ip_h );
	int SetIp( const char * ipstr );
	int SetPort( uint16_t port_h );

	char * GetIpStr( char * buf, uint32_t size )const;
	uint32_t GetIp()const;
	uint16_t GetPort()const;
private:	
	struct Internal;	
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_ADDRIPV4_H__
