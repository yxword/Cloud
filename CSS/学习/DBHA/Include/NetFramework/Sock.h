//
//  "$Id: Sock.h 17071 2010-12-08 05:48:16Z wang_haifeng $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-7		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_H__

#include "SockAddr.h"
#include "NDefs.h"

namespace Dahua{
namespace NetFramework{

const int INVALID_SOCKET_HANDLE = -1;

// 通用套接字抽象类(系统Socket、DHTS Socket)
class NETFRAMEWORK_API CSock
{
public:
	CSock();
	virtual ~CSock();
public:
	//关闭套接字，对象析构时由析构函数调用，无需再调用。
	//返回值：-1 关闭失败，0 关闭成功。
	int Close();
	//返回套接字的类型，具体的类型值见本类定义的枚举量。
	uint32_t GetType() const;

	//设置socket为阻塞模式或非阻塞模式，缺省为非阻塞，一般无需再设置。
	//参数值： true为阻塞，false为非阻塞。
	void SetBlockOpt( bool isblock );
	//查询套接字是否阻塞，
	//返回值：true为阻塞，false为非阻塞。
	bool GetBlockOpt();

	//获取本地地址。
	//返回值：-1 失败，0 成功。
	int GetLocalAddr( CSockAddr *local );
	//获取对端地址。
	//返回值：-1 失败，0 成功。
	int GetRemoteAddr( CSockAddr *remote );

	//检查套接字是否有效。
	//返回值：true 有效 false 无效
	bool IsValid();
	//获取套接字句柄值。
	int GetHandle();

	//把一个套接字句柄与本套接字对象绑定。
	//返回值： -1 失败 0 成功
	int Attach( int sockfd );
	//把本套接字对象上的句柄解绑定。
	//返回值： 被解绑定的句柄。
	int Detach();
	//套接字类型
	enum SOCKET_TYPE{
		SOCK_TYPE_RAW,
		SOCK_TYPE_STREAM,
		SOCK_TYPE_DGRAM,
		SOCK_TYPE_SSL_STREAM,
		SOCK_TYPE_SIMU_STREAM,
		SOCK_TYPE_DHTS,
		SOCK_TYPE_OTHER,
		SOCK_TYPE_UNKNOW,
	};
private:
	//继承类禁止使用成员变量
	int			m_sockfd;
	uint32_t	m_type;
	//套接字是否阻塞，缺省为非阻塞
	bool		m_isblock;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_H__
