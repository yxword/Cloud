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

	//���ͷ��顣
	//����buf��len��ʾ���鼰�䳤�ȡ����ڷ��͵ı��ص�ַ���Զ˵�ַ��MAC��ַ�����������úá�
	//����ֵ��-1 ʧ�ܣ����ڵ���0��ʾ���ͳ�ȥ����������
	int Send( const char * buf, uint32_t len );
	//���շ��顣
	//����buf��len�ǽ��ջ��弰���С��bufֻ����ȥ������̫��Э���ײ�������ݡ�
	//len���ջᱣ��ʵ���յ������ݴ�С��ȥ����̫��Э���ײ���
	//local��remote�ǽ��յ��ķ���ı��غͶԶ�IP��ַ��
	//����ֵ�� -1ʧ�ܣ����ڵ���0��ʾЭ�����ͣ�ETH_P_ARP, ETH_P_RARP, IPPROTO_UDP, IPPROTO_ICMP
	//																				IPPROTO_IGMP���ֵ����֮һ��
	int Recv(char * buf, uint32_t& len, CSockAddr* local = NULL, CSockAddr* remote = NULL,
									unsigned char* local_mac = NULL, unsigned char* remote_mac = NULL );

	// ��ԭʼ�װ��֡�
	// \param [in] ifname ����ӿڵ����ƣ�����eth0,eth0,lo�ȡ�
	// \param [in] protocol Э���������ò������£�
	//					ETH_P_IP		ֻ���շ�������mac��ip���͵�����֡��Ϊȱʡֵ��
	//					ETH_P_ARP		ֻ���ܷ�������mac��arp���͵�����֡
	//					ETH_P_RARP	ֻ���ܷ�������mac��rarp���͵�����֡
	//					ETH_P_ALL		���շ�������mac����������ip arp rarp������֡,
	//											���մӱ����������������͵�����֡.(����ģʽ�򿪵������,
	//											����յ��Ƿ�������mac������֡)
	// \param [in] ip_protocol ��ETH_P_IPʱ��ָ����������������Э�飬֧�ֲ������£�
	//					IPPROTO_UDP	UDPЭ�顣Ϊȱʡֵ��
	//					IPPROTO_ICMP	ICMPЭ��
	//					IPPROTO_IGMP	IGMPЭ��
	//					ip_protocol��ΪETH_P_IPʱ���ò�����Ч��
	//����ֵ��-1ʧ�ܣ�0�ɹ�
	int Open( const char* ifname, uint16_t protocol = ETH_P_IP, uint8_t ip_protocol = IPPROTO_UDP );
	// ��ԭʼ�׽��ֲ�����ָ������(����flags����)
	// \param [in] flags socket��������
	//		0x00  ������,��Open()�ӿ�һ�¡�
	//		0x01  ʹ��PF_PACKET��PF_INET��socket����������IPЭ�飬����*nixϵͳ����Ч
	int Open( const char* ifname, uint16_t protocol, uint8_t ip_protocol, uint32_t flags );
public:
	//���ñ���mac��ַ��ֻ���޸��˷���ȥ��ÿ������Դmac��ַ�������޸�����������
	//����ֵ�� -1ʧ�ܣ�0�ɹ�
	int SetLocalMac( unsigned char * mac_addr );
	//���öԶ�mac��ַ
	//����ֵ�� -1ʧ�ܣ�0�ɹ�
	int SetRemoteMac( unsigned char * mac_addr );
	//���ñ���IP��ַ�Ͷ˿ں�
	//����ֵ��-1ʧ�ܣ�0�ɹ�
	int SetLocalIP( CSockAddrIPv4 &local );
	//���öԶ�IP��ַ�Ͷ˿ں�
	//����ֵ�� -1ʧ�ܣ�0�ɹ�
	int SetRemoteIP( CSockAddrIPv4 &remote );
public:
	//��ȡ����ӿ�������
	//����ֵ�� -1ʧ�ܣ�����0��ֵ��ʾ��ȡ��������ӿ�������
	int GetIFIndex();
	//��ȡ����mac��ַ��
	//bufΪ�����Ļ�������lenΪ���������ȡ�
	//����ֵ�� -1ʧ�ܣ�0�ɹ���
	int GetLocalMac( unsigned char *buf, uint32_t len );
	//��ȡ�Զ�mac��ַ��
	//bufΪ�����Ļ�������lenΪ���������ȡ�
	int GetRemoteMac( unsigned char *buf, uint32_t len );
	//��ȡ����IP��ַ�Ͷ˿ں�
	//����ֵ�� -1ʧ�ܣ�0�ɹ�
	int GetLocalIP( CSockAddrIPv4 &local );
	//��ȡ�Զ�IP��ַ�Ͷ˿ں�
	//����ֵ�� -1ʧ�ܣ�0�ɹ�
	int GetRemoteIP( CSockAddrIPv4 &remote );
public:
	//���ñ���IP��ַ�Ͷ˿ں�
	//����ֵ��-1ʧ�ܣ�0�ɹ�
	int SetLocalIP( CSockAddr* local );
	//���öԶ�IP��ַ�Ͷ˿ں�
	//����ֵ�� -1ʧ�ܣ�0�ɹ�
	int SetRemoteIP( CSockAddr* remote );
private:
	struct Internal;
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SOCK_PACKET_H__
