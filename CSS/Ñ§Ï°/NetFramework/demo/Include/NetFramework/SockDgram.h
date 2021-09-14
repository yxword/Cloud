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

//����UDP�׽��ֵĶ���
class NETFRAMEWORK_API CSockDgram: public CSock
{
	CSockDgram& operator=(CSockDgram const& other);
	CSockDgram(CSockDgram const& other);
public:
	CSockDgram();
	~CSockDgram();
	//����UDP���ݱ�
	//������buf �����͵����ݣ�len ���������ݵĳ���  remote ���ͶԶ˵ĵ�ַ��
	//����ֵ��-1 ʧ�� 0 δ���ͳ�ȥ���´����� ����0��ֵ��ʾ�ѷ��͵ĳ��ȡ�
	int Send( const char * buf, uint32_t len, const CSockAddr * remote = NULL );
	//����UDP���ݱ�
	//������ͬLinuxϵͳ����writev�Ĳ�����
	//����ֵ��-1ʧ�ܣ�0δ�����´����ԣ�����0��ʾ���ͳ�ȥ�ĳ��ȡ� 
	int WriteV( const struct iovec *vector, int count );	
	//����UDP���ݱ�
	//������buf �������ݵĻ��� len �������ݻ���Ĵ�С remote �������ݵĶԶ˵�ַ��
	//����ֵ��-1 ʧ�� 0 δ���յ����´����� ����0��ֵ��ʾ�ѽ��յ��ĳ��ȡ�
	int Recv(char * buf, uint32_t len, CSockAddr* remote = NULL);
	
	//��һ��UDP�׽��֡�
	//������addr ���󶨵ı��ص�ַ��Ҳ�������鲥��ַ����ʾ������鲥�顣
	//����ֵ��-1 ʧ�� 0 �ɹ���
	int Open(const CSockAddr * addr = NULL);
	//��һ��UDP�׽���
	//������addr ���󶨵ı��ص�ַ��Ҳ�������鲥��ַ����ʾ������鲥�顣
	//		reuseaddr �Ƿ�����SO_REUSEADDR,һ�����ڷ������˿�����
	//����ֵ��-1 ʧ�� 0 �ɹ���
	int Open(const CSockAddr *addr, bool reuseaddr );
	//����Զ��ip�Ͷ˿ڣ�����֮����Sendʱ���Բ�������Զ�˵�ַ
	//���� :	remote�������ݵĶԶ˵�ַ
	//����ֵ:-1ʧ�� 0�ɹ�
	int SetRemote( const CSockAddr* remote );
public:
	//���½ӿڶ��Ƕಥ��صģ����������Ϊ�����ֽ���
	//���öಥ���͵ı��ؽӿ�
	//����ֵ��-1 ʧ�� 0 �ɹ�
	int SetMulticastIF( uint32_t ip_h );
	int SetMulticastIF( const char * ipstr );
	//���öಥ���ͱ��ĵ�TTL
	//����ֵ�� -1 ʧ�� 0 �ɹ�
	int SetMulticastTTL( uint8_t TTL );
	//�����Ƿ��ֹ�鲥���ݻ��ͣ�true-���ͣ�false-�����͡�
	//����ֵ: -1 ʧ�� 0 �ɹ�
	int SetMulticastLoop( bool isloop );
private:
	struct Internal;	
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_DGRAM_H__
