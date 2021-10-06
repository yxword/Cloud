//
//  "$Id: TcpSvr.h 7672 2010-5-17 02:28:18Z he_linqiang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-5-17        he_linqiang Create
//

#ifndef __INCLUDED_TCP_SVR_H__
#define __INCLUDED_TCP_SVR_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SockAcceptor.h"
#include "NetFramework/SockStream.h"
using namespace Dahua::NetFramework;

class CTcpSvr : public CNetHandler
{
public:
	CTcpSvr();
	virtual ~CTcpSvr();
public:
	int StartSvr( CSockAddrIPv4 & addr );
public:
	//ע�⣬��ͬ���׽��֣���ʱ���Ļص��������п��ܱ�ͬʱ�ص���Ҫ���ù������ݵĻ�����ʡ�
	virtual int handle_input( int handle );
	virtual int handle_input_timeout( int handle );
	virtual int handle_close(CNetHandler *myself);
private:
	//���մ�m_acceptor�����������׽��֣�Ҫ��ָ�룬��Ҫ�����ڴ���ͷš�
	CSockStream*	m_stream;
	CSockAcceptor	m_acceptor;
};

int StartTcpSvr();

#endif //__INCLUDED_TCP_SVR_H__
