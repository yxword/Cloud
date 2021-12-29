//
//  "$Id: SockAddrIPv6.h 56265 2012-03-30 04:58:02Z zhou_mingwei $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-5-7        he_linqiang Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_ADDRIPV6_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_ADDRIPV6_H__

#include "SockAddr.h"
#include "NDefs.h"
#ifdef WIN32
#ifdef SetPort
#undef SetPort
#endif  //endif SetPort
#include "NTypes_Win32.h"
#else
#include <netinet/in.h>
#endif

namespace Dahua{
namespace NetFramework{


class NETFRAMEWORK_API CSockAddrIPv6 : public CSockAddr
{
public:
	CSockAddrIPv6();
	~CSockAddrIPv6();
	CSockAddrIPv6( const char * ipstr, uint16_t port_h );
	CSockAddrIPv6( const char * ipstr, uint16_t port_h, uint32_t scope_id );
	CSockAddrIPv6( const struct sockaddr_in6 * addr );
	CSockAddrIPv6(CSockAddrIPv6 const& other);
	bool operator==( CSockAddrIPv6 & other)const;
	CSockAddrIPv6& operator=( CSockAddrIPv6 const & other );
public:
	virtual bool IsValid() const;
	virtual struct sockaddr * GetAddr(  struct sockaddr * addr ) const;
	virtual uint32_t GetAddrLen() const;
public:
	int SetAddr( const char * ipstr, uint16_t port_h );
	int SetAddr( const char * ipstr, uint16_t port_h, uint32_t scope_id );
	int SetAddr( const struct sockaddr_in6 * addr );
	int SetIp( const char * ipstr );
	int SetPort( uint16_t port_h );
	char * GetIpStr( char * buf, uint32_t size ) const;
	uint16_t GetPort() const;
	int SetScopeID( uint32_t scope_id );
	uint32_t GetScopeID() const;
private:
	struct Internal;
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_ADDRIPV6_H__
