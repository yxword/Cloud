//
//  "$Id: SockPacket.h 55658 2012-03-26 08:46:07Z zhou_mingwei $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-25		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SOCK_PACKET_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SOCK_PACKET_H__

#ifdef WIN32
#include "Infra/IntTypes.h"
#else
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <netinet/udp.h>
#include <endian.h>
#endif
#include <string.h>
#include "SockAddrIPv4.h"
#include "Sock.h"

namespace Dahua{
namespace NetFramework{

class NETFRAMEWORK_API CSockPacket : public CSock
{
	CSockPacket& operator=(CSockPacket const& other);
	CSockPacket(CSockPacket const& other);
public:
	CSockPacket();
	~CSockPacket();

	//发送分组。
	//参数buf和len表示分组及其长度。关于发送的本地地址，对端地址，MAC地址等需事先设置好。
	//返回值：-1 失败，大于等于0表示发送出去的数据量。
	int Send( const char * buf, uint32_t len );
	//接收分组。
	//参数buf和len是接收缓冲及其大小，buf只返回去掉了以太网协议首部后的数据。
	//len最终会保存实际收到的数据大小（去除以太网协议首部）
	//local和remote是接收到的分组的本地和对端IP地址。
	//返回值： -1失败，大于等于0表示协议类型：ETH_P_ARP, ETH_P_RARP, IPPROTO_UDP, IPPROTO_ICMP
	//																				IPPROTO_IGMP五个值其中之一。
	int Recv(char * buf, uint32_t& len, CSockAddr* local = NULL, CSockAddr* remote = NULL,
									unsigned char* local_mac = NULL, unsigned char* remote_mac = NULL );

	// 打开原始套按字。
	// \param [in] ifname 网络接口的名称，比如eth0,eth0,lo等。
	// \param [in] protocol 协议名，可用参数如下：
	//					ETH_P_IP		只接收发往本机mac的ip类型的数据帧，为缺省值。
	//					ETH_P_ARP		只接受发往本机mac的arp类型的数据帧
	//					ETH_P_RARP	只接受发往本机mac的rarp类型的数据帧
	//					ETH_P_ALL		接收发往本机mac的所有类型ip arp rarp的数据帧,
	//											接收从本机发出的所有类型的数据帧.(混杂模式打开的情况下,
	//											会接收到非发往本地mac的数据帧)
	// \param [in] ip_protocol 当ETH_P_IP时，指定具体的网络层或传输层协议，支持参数如下：
	//					IPPROTO_UDP	UDP协议。为缺省值。
	//					IPPROTO_ICMP	ICMP协议
	//					IPPROTO_IGMP	IGMP协议
	//					ip_protocol不为ETH_P_IP时，该参数无效。
	//返回值：-1失败，0成功
	int Open( const char* ifname, uint16_t protocol = ETH_P_IP, uint8_t ip_protocol = IPPROTO_UDP );
	// 打开原始套接字并启用指定特性(增加flags参数)
	// \param [in] flags socket属性设置
	//		0x00  无设置,和Open()接口一致。
	//		0x01  使用PF_PACKET起到PF_INET的socket类型来承载IP协议，仅在*nix系统下有效
	int Open( const char* ifname, uint16_t protocol, uint8_t ip_protocol, uint32_t flags );
public:
	//设置本地mac地址，只是修改了发出去的每个包的源mac地址，不会修改网卡。慎用
	//返回值： -1失败，0成功
	int SetLocalMac( unsigned char * mac_addr );
	//设置对端mac地址
	//返回值： -1失败，0成功
	int SetRemoteMac( unsigned char * mac_addr );
	//设置本地IP地址和端口号
	//返回值：-1失败，0成功
	int SetLocalIP( CSockAddrIPv4 &local );
	//设置对端IP地址和端口号
	//返回值： -1失败，0成功
	int SetRemoteIP( CSockAddrIPv4 &remote );
public:
	//获取网络接口索引号
	//返回值： -1失败，大于0的值表示获取到的网络接口索引号
	int GetIFIndex();
	//获取本地mac地址。
	//buf为待填充的缓冲区，len为缓冲区长度。
	//返回值： -1失败，0成功。
	int GetLocalMac( unsigned char *buf, uint32_t len );
	//获取对端mac地址。
	//buf为待填充的缓冲区，len为缓冲区长度。
	int GetRemoteMac( unsigned char *buf, uint32_t len );
	//获取本地IP地址和端口号
	//返回值： -1失败，0成功
	int GetLocalIP( CSockAddrIPv4 &local );
	//获取对端IP地址和端口号
	//返回值： -1失败，0成功
	int GetRemoteIP( CSockAddrIPv4 &remote );
public:
	//设置本地IP地址和端口号
	//返回值：-1失败，0成功
	int SetLocalIP( CSockAddr* local );
	//设置对端IP地址和端口号
	//返回值： -1失败，0成功
	int SetRemoteIP( CSockAddr* remote );
private:
	struct Internal;
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SOCK_PACKET_H__
