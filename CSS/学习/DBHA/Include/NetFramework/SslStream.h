//
//  "$Id: SslStream.h 7672 2010-5-27 02:28:18Z zmw $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-27		zmw         	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SSL_STREAM_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SSL_STREAM_H__

#include "NDefs.h"
#include "Infra/IntTypes.h"
#include "SockAddr.h"
#include "SockStream.h"
#include "SslX509.h"
#ifndef WIN32
#include <sys/uio.h>
#endif

namespace Dahua{
namespace NetFramework{

class NETFRAMEWORK_API CSslStream : public CSock
{
	CSslStream& operator=(CSslStream const& other);
	CSslStream(CSslStream const& other);
public:
	CSslStream();
	~CSslStream();
public:
/// �Ͻӿڣ�ǰ����ݡ�
	//����ssl��Կ�ļ�cacert.pem��privkey.pem��·������Ҫ����Ϊϵͳ·��
	//������path ϵͳ��Կ�ļ�cacert.pem��privkey.pem�ľ���·��
	//����ֵ��-1 ʧ�� 0 �ɹ�
	int SetPemPath( const char* path );
	// ��ȡSSL�����У��Զ�Я��������֤����Ϣ
	//����ֵ�� <0 ʧ�� 0 �ɹ�
	int GetPeerCert( CSslX509* x509 );
public:
	//����ssl����Զ�˷�����
	//������remote Զ�˷���˵ĵ�ַ������IP�Ͷ˿�
	//����ֵ��-1 ʧ��  0 �ɹ�
	int Connect(const CSockAddr & remote,CSockAddr* local = NULL);
	//���Ѿ�TCP��������½���ssl����
	//������sock �Ѿ�����TCP���ӵ�CSockStreamָ��
	//����ֵ��-1ʧ�� 0 �ɹ�
	int Connect( CSockStream* sock );
	//��������
	//������buf �����ͻ����� len ���������ݵ��ֽ���
	//����ֵ��-1 ʧ�� 0 δ���ͳ�ȥ ����0��ֵ��ʾ�ѷ��͵ĳ���
	int Send( const char * buf, uint32_t len );
	//��������
	//������buf �������ݵĻ��� len �������ݻ���Ĵ�С
	//����ֵ��-1 ʧ�� 0 δ�������� ����0 �ѽ��������ֽ���
	int Recv( char * buf, uint32_t len );
	//����TCP���ݣ�
	//������ͬLinuxϵͳ����writev�Ĳ�����
	//����ֵ��-1ʧ�ܣ�0δ�����´����ԣ�����0��ʾ���ͳ�ȥ�ĳ��ȡ�
	int WriteV( const struct iovec *vector, int count );
	//���г�ʱ���������ͣ����գ�ͬ����Ҫ�����׽���Ϊ������ģʽ��
	//��ʱʱ�䵥λΪ΢�룬ȱʡΪ1�롣
	//������buf �����ͻ����� len ���������ݵ��ֽ��� timeout ��ʱʱ�䣬Ĭ��Ϊ1��
	//����ֵ��-1 ʧ�� 0  δ�������� ����0 ��ʱʱ���ڷ��͵�������
	int Send_n( const char * buf, uint32_t len, int timeout = 1000000 );
	//����ʱ�Ľ���
	//������buf ���ջ����� len ���ջ��������� timeout ��ʱʱ�䣬Ĭ��1��
	//����ֵ��-1 ʧ�� 0 δ���յ����� ����0 ʵ�ʽ��յ���������
	int Recv_n( char * buf, uint32_t len, int timeout = 1000000 );
	//��TCPջ�л�ȡ������SSL�������������ص�ǰ���峤�ȡ����δ������SSL������peekֵ���ȵ����ݣ������ٴ�TCPջ�н��������ݡ�
	//�ýӿڽ���Recv()�ӿڽ��ʹ��, ��Recv()֮ǰʹ�á�
	//����:  ��
	//����ֵ��-1 ʧ��, >=0 SSL�����������ݵĳ���
	int Peek();
public:
	//����SSL���ӵ�����,��Connect����֮ǰʹ�á�
	//�������Ͻӿ�,�������¹���,����compatibility��
	//������option  ����������
	//		type    ���������
	//      para    ���������
	int SetOption(int option, int type, void* para);
	/// ���� para�ĳ���ֵ,��������Ʋ������Ȳ�ȷ������
	// ����SSL���ӵ�����,��Connect����֮ǰʹ�á�
	///\param[in] option  ����������
	///\param[in] type    ���������
	///\param[in] para    ���������
	///\param[in] len     ���Բ����ڴ�鳤��
	int SetOption(int option, int type, void* para, int len);
	/// ��ȡSSL���ӵ�����,�Ƽ������ӽ�����֮��ʹ��
	///\param[in] option  ����������
	///\param[in/out] type    ���������
	///\param[out] para   ������������ڴ��ɵ����߷���
	///\param[in/out] len �����ڴ�鳤��/ʵ���ڴ�鳤��
	int GetOption(int option, int& type, void* para, int& len);
private:
	struct Internal*	m_internal;
};

}//NetFramework
}//Dahua

#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SSL_STREAM_H__

