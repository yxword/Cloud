//
//  "$Id: SslSvr.h 7672 2010-5-28 02:28:18Z zmw $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-5-28        zmw				 Create
//

#ifndef __INCLUDED_SSL_SVR_H__
#define __INCLUDED_SSL_SVR_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SslAcceptor.h"
#include "NetFramework/SslStream.h"

using namespace Dahua::NetFramework;

#ifdef _USE_OPENSSL

class CSslSvr : public CNetHandler
{
public:
	CSslSvr();
	virtual ~CSslSvr();
public:
	int  StartSvr( CSockAddrIPv4 & addr, int session );
	void StopSvr();
public:
	//ע�⣬��ͬ���׽��֣���ʱ���Ļص��������п��ܱ�ͬʱ�ص���Ҫ���ù������ݵĻ�����ʡ�
	virtual int handle_input( int handle );
	virtual int handle_input_timeout( int handle );
	
	virtual int handle_close(CNetHandler *myself);
private:
	//���մ�m_acceptor�����������׽��֣�Ҫ��ָ�룬��Ҫ�����ڴ���ͷš�
	CSslStream*	m_stream;
	CSslAcceptor	m_acceptor;
};

int StartSslSvr(int argc, char **argv);

#endif

#endif //__INCLUDED_SSL_SVR_H__
