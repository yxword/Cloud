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

#ifndef __INCLUDED_NB_SSL_SVR_H__
#define __INCLUDED_NB_SSL_SVR_H__

#include <map>
#include <list>
#include "Infra/Mutex.h"

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SslAsyncAcceptor.h"
#include "NetFramework/SslAsyncStream.h"


using namespace Dahua::Infra;
using namespace Dahua::NetFramework;

#ifdef _USE_OPENSSL

class CNbSslSvr : public CNetHandler
{
public:
	CNbSslSvr();
	virtual ~CNbSslSvr();
public:
	int StartSvr( CSockAddrIPv4 & addr, int session );
public:
	//ע�⣬��ͬ���׽��֣���ʱ���Ļص��������п��ܱ�ͬʱ�ص���Ҫ���ù������ݵĻ�����ʡ�
	virtual int handle_input( int handle );
	virtual int handle_input_timeout( int handle );

	virtual int handle_close(CNetHandler *myself);
	
private:
	int processConnRequest(int handle);
private:
	//���մ�m_acceptor�����������׽��֣�Ҫ��ָ�룬��Ҫ�����ڴ���ͷš�
	Dahua::Infra::CMutex	   m_mutex;
	std::map<int, CSslAsyncStream*> m_list;
	CSslAsyncAcceptor	m_acceptor;
};

int StartNbSslSvr(int argc, char **argv);

#endif

#endif //__INCLUDED_NB_SSL_SVR_H__
