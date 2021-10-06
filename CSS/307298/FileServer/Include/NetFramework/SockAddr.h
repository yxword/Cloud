//
//  "$Id: SockAddr.h 48672 2012-02-03 03:13:11Z zhou_mingwei $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-7		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SOCK_ADDR_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SOCK_ADDR_H__

#include "Infra/IntTypes.h"
#ifdef WIN32
#include "NTypes_Win32.h"
#else
#include <sys/socket.h>
#endif
#include "NDefs.h"

namespace Dahua{
namespace NetFramework{

class NETFRAMEWORK_API CSockAddr{
public:
	CSockAddr();
	virtual ~CSockAddr();
public:
	//获取类型为struct sockaddr的地址，返回的指针即为传入参数addr的地址。
	virtual struct sockaddr * GetAddr(  struct sockaddr * addr ) const = 0;
	//获取GetAddr获取的地址的长度。
	virtual uint32_t GetAddrLen() const = 0;

	//当前地址的类型，据此类型可以向派生类作类型转换
	uint32_t GetType() const;
	//当前地址是否为有效的地址对象。
	virtual bool IsValid()const = 0;
	//在大华的各种设备中，很多程序都使用固定分配的端口。这就表示除了低于1024的系统保
	//留端口，大华设备也保留了一定数量的端口，我们在编写应用的时候，使用的临时端口要
	//确保不和这些保留端口冲突，这个接口用于查询当前的网络地址是否为已保留的地址。
	//返回值：true表示已保留，false表示未保留。
	bool IsReserved();
	/*SOCKADDR_TYPE_UNKNOWN		无效地址类型
	  SOCKADDR_TYPE_IPV4		AF_INET域地址类型
	  SOCKADDR_TYPE_IPV6		AF_INET6域地址类型
	  SOCKADDR_TYPE_STORAGE		地址存储类型，能存储AF_INET域和AF_INET6域地址类型
	  SOCKADDR_TYPE_UN			AF_UNIX域地址类型，提供这个只是为以后的地址类型扩展作个示例，当前无用处。
	  */
	enum SOCKADDR_TYPE{ 
		SOCKADDR_TYPE_UNKNOWN, 
		SOCKADDR_TYPE_IPV4,
		SOCKADDR_TYPE_IPV6, 
		SOCKADDR_TYPE_STORAGE,
		SOCKADDR_TYPE_UN 
	};
protected:
	uint32_t m_type;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SOCK_ADDR_H__
