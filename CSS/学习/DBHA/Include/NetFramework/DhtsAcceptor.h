//
//  "$Id: DhtsAcceptor.h 7672 2010-5-10 02:28:18Z rao_ping $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2017-12-10		rao_ping	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_DHTS_ACCEPTOR_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_DHTS_ACCEPTOR_H__

#include "NDefs.h"
#include "DhtsStream.h"
#include "SockAddr.h"
#include <string.h>

namespace Dahua{
namespace NetFramework{

//����һ��TCP�����׽��֡�
class NETFRAMEWORK_API CDhtsAcceptor : public CSock 
{
	CDhtsAcceptor( CDhtsAcceptor const & other );
	CDhtsAcceptor& operator=( CDhtsAcceptor const & other );
public:
	CDhtsAcceptor();
	~CDhtsAcceptor();
public:
	//��һ���������С�
	//������local���������ı��ص�ַ��
	//����ֵ��-1ʧ�ܣ�0�ɹ�
	int Open( const CSockAddr & local );
	//����һ��Զ����������
	//������remote���������Զ�˵�ַ��
	//����ֵ�� NULL����ʧ�� ָ����Ч��Ϊ���ճɹ����һ����ַ�׽��֣�ָ���ɵ����߹��������ͷš�
	CDhtsStream* Accept( CSockAddr * remote = NULL );
	//����һ��Զ�˵���������
	//������sock���������ܵ���������
	//		remoteԶ�˵�ַ
	//����ֵ��-1������ʧ�ܣ�sock������ 0���ɹ���sock����
	int Accept( CDhtsStream& sock, CSockAddr * remote = NULL );
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_DHTS_ACCEPTOR_H__

