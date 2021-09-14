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
	//注意，不同的套接字，或定时器的回调函数都有可能被同时回调，要作好共享数据的互斥访问。
	virtual int handle_input( int handle );
	virtual int handle_input_timeout( int handle );

	virtual int handle_close(CNetHandler *myself);
	
private:
	int processConnRequest(int handle);
private:
	//接收从m_acceptor中侦听到的套接字，要用指针，且要负责内存的释放。
	Dahua::Infra::CMutex	   m_mutex;
	std::map<int, CSslAsyncStream*> m_list;
	CSslAsyncAcceptor	m_acceptor;
};

int StartNbSslSvr(int argc, char **argv);

#endif

#endif //__INCLUDED_NB_SSL_SVR_H__
