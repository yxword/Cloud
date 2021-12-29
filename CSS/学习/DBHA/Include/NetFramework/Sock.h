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

// ͨ���׽��ֳ�����(ϵͳSocket��DHTS Socket)
class NETFRAMEWORK_API CSock
{
public:
	CSock();
	virtual ~CSock();
public:
	//�ر��׽��֣���������ʱ�������������ã������ٵ��á�
	//����ֵ��-1 �ر�ʧ�ܣ�0 �رճɹ���
	int Close();
	//�����׽��ֵ����ͣ����������ֵ�����ඨ���ö������
	uint32_t GetType() const;

	//����socketΪ����ģʽ�������ģʽ��ȱʡΪ��������һ�����������á�
	//����ֵ�� trueΪ������falseΪ��������
	void SetBlockOpt( bool isblock );
	//��ѯ�׽����Ƿ�������
	//����ֵ��trueΪ������falseΪ��������
	bool GetBlockOpt();

	//��ȡ���ص�ַ��
	//����ֵ��-1 ʧ�ܣ�0 �ɹ���
	int GetLocalAddr( CSockAddr *local );
	//��ȡ�Զ˵�ַ��
	//����ֵ��-1 ʧ�ܣ�0 �ɹ���
	int GetRemoteAddr( CSockAddr *remote );

	//����׽����Ƿ���Ч��
	//����ֵ��true ��Ч false ��Ч
	bool IsValid();
	//��ȡ�׽��־��ֵ��
	int GetHandle();

	//��һ���׽��־���뱾�׽��ֶ���󶨡�
	//����ֵ�� -1 ʧ�� 0 �ɹ�
	int Attach( int sockfd );
	//�ѱ��׽��ֶ����ϵľ����󶨡�
	//����ֵ�� ����󶨵ľ����
	int Detach();
	//�׽�������
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
	//�̳����ֹʹ�ó�Ա����
	int			m_sockfd;
	uint32_t	m_type;
	//�׽����Ƿ�������ȱʡΪ������
	bool		m_isblock;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_H__
