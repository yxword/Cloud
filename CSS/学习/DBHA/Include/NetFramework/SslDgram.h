//
//  "$Id:"
//
//  Copyright (c)1992-2015, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2015-8-15		 he_wei  	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SSL_DGRAM_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SSL_DGRAM_H__

#include "NDefs.h"
#include "SockAddr.h"
#include "SockDgram.h"

namespace Dahua {
namespace NetFramework{

class NETFRAMEWORK_API CSslDgram : public CSock
{
public:
	CSslDgram();
	~CSslDgram();
public:
	// �ýӿ�����ticket-session�����ĳ�ʼ������������һ���ticket���мӽ��ܵ���Կ��
	// ����ֵ: 0
	static int Init();
public:
///������
	/// ���������ӿ�����ִ�з�������Accept�Ĳ�����
	//�򿪷���˵ı��ض˿�
	//����: addr ���ض˿ڵ�ַ�� reuseaddr �ö˿��Ƿ������
	//����ֵ: -1 ����ʧ��  0 �����ɹ�
	int Open( const CSockAddr *addr, bool reuseaddr = false );
	//���ڷ�������ִ�нӿ������������
	//����: remote �Ƿ�ָ�������ַ
	//����ֵ��-1 ����ʧ�� 0 �����ɹ�
	int AcceptX( const CSockAddr *remote = NULL );
///����
	// �����Ľӿ�Զ�˵���������
	// ����: addr ���ؼ����˿ڣ� reuseaddr �ö˿��Ƿ������ remote��ָ���������ַ
	// ����ֵ: -1 ����ʧ�� 0 �����ɹ�
	int Accept( const CSockAddr *addr, bool reuseaddr = false, const CSockAddr *remote = NULL );
public:
	// ������SSL/TLS���� ( �������� )
	// ����: remote��Զ�̷���˵�ַ��local ���ص�ַ
	// ����ֵ: -1 ����ʧ�ܣ� 0 ���ӳɹ�
	int Connect( const CSockAddr *remote, const CSockAddr* local = NULL );
public:
	// ����SSL/TLS����ѡ��
	// ����: option ������ type ���������� para ���������
	// ����ֵ: 0
	int SetOption(int option, int type, void* para);
public:
	//����UDP���ݱ�
	//������buf �����͵����ݣ�len ���������ݵĳ���
	//����ֵ��-1 ʧ�� 0 δ���ͳ�ȥ���´����� ����0��ֵ��ʾ�ѷ��͵ĳ��ȡ�
	int Send( const char * buf, uint32_t len );
	//����UDP���ݱ�
	//������buf �������ݵĻ��� len �������ݻ���Ĵ�С
	//����ֵ��-1 ʧ�� 0 δ���յ����´����� ����0��ֵ��ʾ�ѽ��յ��ĳ��ȡ�
	int Recv(char * buf, uint32_t len );
private:
	struct Internal*	m_internal;
};

}; // NetFramework
}; // Dahua

#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SSL_DGRAM_H__
