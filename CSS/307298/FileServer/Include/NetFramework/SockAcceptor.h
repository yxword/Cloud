//
//  "$Id: SockAcceptor.h 30412 2011-07-21 05:57:44Z qin_fenglin $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-10		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_ACCEPTOR_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_ACCEPTOR_H__

#include "SockStream.h"
#include "SockAddr.h"
#include "NDefs.h"
#include <string.h>

namespace Dahua{
namespace NetFramework{

//����һ��TCP�����׽��֡�
class NETFRAMEWORK_API CSockAcceptor : public CSock 
{
	CSockAcceptor( CSockAcceptor const & other );
	CSockAcceptor& operator=( CSockAcceptor const & other );
public:
	CSockAcceptor();
	~CSockAcceptor();
public:
	//��һ���������С�
	//������local���������ı��ص�ַ��
	//����ֵ��-1ʧ�ܣ�0�ɹ�
	int Open( const CSockAddr & local );
#ifndef WIN32
	//����һ��Զ����������
	//������remote���������Զ�˵�ַ��
	//����ֵ�� NULL����ʧ�� ָ����Ч��Ϊ���ճɹ����һ����ַ�׽��֣�ָ���ɵ����߹��������ͷš�
	CSockStream* Accept( CSockAddr * remote = NULL );
#endif
	//����һ��Զ�˵���������
	//������sock���������ܵ���������
	//		remoteԶ�˵�ַ
	//����ֵ��-1������ʧ�ܣ�sock������ 0���ɹ���sock����
	int Accept( CSockStream& sock, CSockAddr * remote = NULL );
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SOCKET_ACCEPTOR_H__
