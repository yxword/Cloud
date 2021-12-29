//
//  "$Id: SockAddrIPv6.h 7672 2010-5-7 02:28:18Z he_linqiang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-5-7        he_linqiang Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_ADDRSTORAGE_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_ADDRSTORAGE_H__

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

/* IP地址存储类，用于在未知地址类型的情况下，输入IP地址、Port获取准确类型时使用，
 * 也用于通过网络获取地址时使用，如CSockAcceptor::Accept(CSockAddrStorage)。在有
 * 确定存储IP地址和端口时，也可以作为基本地址类型使用，如CSockStream::Connet(CSockAddrStorage)
 * 此时地址类型以输入的形式为准，如IP地址为::ffff:1.2.3.4，发起的是IPv6的行为
 */
 
namespace Dahua{
namespace NetFramework{

class NETFRAMEWORK_API CSockAddrStorage : public CSockAddr
{	
public:
	CSockAddrStorage();
	~CSockAddrStorage();
	CSockAddrStorage( const char * ipstr, uint16_t port_h );	
	CSockAddrStorage(CSockAddrStorage const& other);
	bool operator==( CSockAddrStorage & other)const;
	CSockAddrStorage& operator=( CSockAddrStorage const & other );
public:
	//存储类保存了确定域的地址后有效
	virtual bool IsValid() const;
	//获取的类型可能为sockaddr_in、sockaddr_in6，由存储的地址类型决定
	//参数：addr：输入保存地址的结构体指针，需要保证addr的类型大小大于	
	//			  等于本类保存的地址类型
	//返回值：NULL，失败，非NULL，sockaddr指针
	virtual struct sockaddr * GetAddr(  struct sockaddr * addr ) const;	
	//获取地址结构体长度，即上一个接口返回地址结构体的长度
	virtual uint32_t GetAddrLen() const;
public:
	//设置地址
	//参数：ipstr：ip字符串或域名
	//		port_h：端口
	//返回值：-1，失败，0，成功
	int SetAddr( const char * ipstr, uint16_t port_h );	
	//可以将sockaddr_in6类型强转后输入，以支持IPv6地址
	//参数：addr：地址指针
	//返回值：-1，失败，0，成功
	int SetAddr( const struct sockaddr_in* addr );
	//设置IP地址
	//参数：ipstr：ip地址字符串，可以为域名
	//返回值：-1，失败，0，成功
	int SetIp( const char * ipstr );
	//设置端口
	//参数：port_h：端口
	//返回值：-1，失败，0，成功
	int SetPort( uint16_t port_h );
	//获取IP地址，为设置输入的地址，若需要获取真实IP地址，
	//需要转换成真实地址类型后再获取
	//参数：buf：保存IP地址字符串，可能为域名
	//		size：buf长度
	//返回值：ip字符串指针
	char * GetIpStr( char * buf, uint32_t size ) const;
	//获取端口地址
	//返回值：0，失败，非0，端口地址
	uint16_t GetPort()const;
	//获取存储的实际地址类型，返回值为SOCKADDR_TYPE类型
	uint32_t GetRealType()const;   
	//将存储的地址转换成实际地址类型，CSockAddrIPv4或CSockAddrIPv6
	//参数：addr：保存转换后的地址类
	//返回值：true，成功，false，失败
	bool GetRealSockAddr( CSockAddr* addr );
	//获取输入的地址类型，CSockAddrIPv4或CSockAddrIPv6(包括IPV4的映射地址)?
	//返回值：SOCKADDR_TYPE
	uint32_t GetInputType()const;
private:
	struct Internal;
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_ADDRSTORAGE_H__
