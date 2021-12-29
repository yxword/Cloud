//
//  "$Id: SslAcceptor.h 7672 2010-5-27 02:28:18Z zmw $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-27		zmw        	Create
//					2015-12-4       hw          Rebuild

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SSL_ACCEPTOR_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SSL_ACCEPTOR_H__

#include "NDefs.h"
#include "SslStream.h"
#include "SockAddr.h"
#include <string.h>

namespace Dahua{
namespace NetFramework{

class NETFRAMEWORK_API CSslAcceptor : public CSock
{
	CSslAcceptor& operator=(CSslAcceptor const& other);
	CSslAcceptor(CSslAcceptor const& other);
public:
	CSslAcceptor();
	~CSslAcceptor();
public:
/// �Ͻӿ�,ǰ����ݡ�
	//����ssl��Կ�ļ�cacert.pem��privkey.pem��·������Ҫ����Ϊϵͳ·��
	//������path ϵͳ��Կ�ļ�cacert.pem��privkey.pem�ľ���·��
	//����ֵ��-1 ʧ�� 0 �ɹ�
	int SetPemPath( const char* path );
	//�����Ƿ���֤�Զ˵�����֤��, ��Accept()֮ǰ����
	//�ýӿڵ����ȼ�����SetOption,�뾡������ʹ�øýӿ�
	//����: flag  �Ƿ���֤�Զ�����֤��
	void SetCertVerityPeer( bool flag );
public:
	//���������������ض˿�
	//������local ���ص�ַ��Ϣ��ip�Ͷ˿�
	//����ֵ��-1ʧ��  0 �ɹ�
	int Open( const CSockAddr & local );
	//����һ��Զ�˵�����������������CSslStream,�ⲿ��Ҫ�������CSslStream.
	//������remote ���ú��¼��������Ŀͻ��˵�ַ��Ϣ
	//����ֵ��CSslStream����ָ�룬NULL ʧ�ܣ�����ɹ�
	CSslStream* Accept( CSockAddr * remote = NULL );
	//����һ��Զ�˵���������, ��Ҫ�ⲿ����CSslStream.
	//������sock���������ܵ���������
	//		remoteԶ�˵�ַ
	//����ֵ��-1������ʧ�ܣ�sock������ 0���ɹ���sock����
	int Accept( CSslStream& sock, CSockAddr * remote = NULL );

	//����һ��Զ�˵�SSL��������,���õ���Open()
	//������sock ���������ܵ���������
	//		stream �Ѿ�����TCP���ӵ�CSockStreamָ��
	//����ֵ��-1������ʧ�ܣ�sock������ 0���ɹ���sock����
	int Accept(CSslStream& sock, CSockStream* stream);
public:
	//����SSL��������ѡ��,����Accept����ǰʹ�á�
	//�������Ͻӿ�,�������¹���,����compatibility��
	//����: option  ������  type �����õ����� para ��������صĲ���
	//����ֵ: 0
	int SetOption(int option, int type, void* para);
	/// ���� para�ĳ���ֵ,��������Ʋ������Ȳ�ȷ������
	// ����SSL���ӵ�����,��Connect����֮ǰʹ�á�
	///\param[in] option  ����������
	///\param[in] type    ���������
	///\param[in] para    ���������
	///\param[in] len     ���Բ����ڴ�鳤��
	int SetOption(int option, int type, void* para, int len);
private:
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua

#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SSL_ACCEPTOR_H__
