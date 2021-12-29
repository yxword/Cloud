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

/* IP��ַ�洢�࣬������δ֪��ַ���͵�����£�����IP��ַ��Port��ȡ׼ȷ����ʱʹ�ã�
 * Ҳ����ͨ�������ȡ��ַʱʹ�ã���CSockAcceptor::Accept(CSockAddrStorage)������
 * ȷ���洢IP��ַ�Ͷ˿�ʱ��Ҳ������Ϊ������ַ����ʹ�ã���CSockStream::Connet(CSockAddrStorage)
 * ��ʱ��ַ�������������ʽΪ׼����IP��ַΪ::ffff:1.2.3.4���������IPv6����Ϊ
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
	//�洢�ౣ����ȷ����ĵ�ַ����Ч
	virtual bool IsValid() const;
	//��ȡ�����Ϳ���Ϊsockaddr_in��sockaddr_in6���ɴ洢�ĵ�ַ���;���
	//������addr�����뱣���ַ�Ľṹ��ָ�룬��Ҫ��֤addr�����ʹ�С����	
	//			  ���ڱ��ౣ��ĵ�ַ����
	//����ֵ��NULL��ʧ�ܣ���NULL��sockaddrָ��
	virtual struct sockaddr * GetAddr(  struct sockaddr * addr ) const;	
	//��ȡ��ַ�ṹ�峤�ȣ�����һ���ӿڷ��ص�ַ�ṹ��ĳ���
	virtual uint32_t GetAddrLen() const;
public:
	//���õ�ַ
	//������ipstr��ip�ַ���������
	//		port_h���˿�
	//����ֵ��-1��ʧ�ܣ�0���ɹ�
	int SetAddr( const char * ipstr, uint16_t port_h );	
	//���Խ�sockaddr_in6����ǿת�����룬��֧��IPv6��ַ
	//������addr����ַָ��
	//����ֵ��-1��ʧ�ܣ�0���ɹ�
	int SetAddr( const struct sockaddr_in* addr );
	//����IP��ַ
	//������ipstr��ip��ַ�ַ���������Ϊ����
	//����ֵ��-1��ʧ�ܣ�0���ɹ�
	int SetIp( const char * ipstr );
	//���ö˿�
	//������port_h���˿�
	//����ֵ��-1��ʧ�ܣ�0���ɹ�
	int SetPort( uint16_t port_h );
	//��ȡIP��ַ��Ϊ��������ĵ�ַ������Ҫ��ȡ��ʵIP��ַ��
	//��Ҫת������ʵ��ַ���ͺ��ٻ�ȡ
	//������buf������IP��ַ�ַ���������Ϊ����
	//		size��buf����
	//����ֵ��ip�ַ���ָ��
	char * GetIpStr( char * buf, uint32_t size ) const;
	//��ȡ�˿ڵ�ַ
	//����ֵ��0��ʧ�ܣ���0���˿ڵ�ַ
	uint16_t GetPort()const;
	//��ȡ�洢��ʵ�ʵ�ַ���ͣ�����ֵΪSOCKADDR_TYPE����
	uint32_t GetRealType()const;   
	//���洢�ĵ�ַת����ʵ�ʵ�ַ���ͣ�CSockAddrIPv4��CSockAddrIPv6
	//������addr������ת����ĵ�ַ��
	//����ֵ��true���ɹ���false��ʧ��
	bool GetRealSockAddr( CSockAddr* addr );
	//��ȡ����ĵ�ַ���ͣ�CSockAddrIPv4��CSockAddrIPv6(����IPV4��ӳ���ַ)?
	//����ֵ��SOCKADDR_TYPE
	uint32_t GetInputType()const;
private:
	struct Internal;
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_ADDRSTORAGE_H__
