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

#ifndef __INCLUDED_SOCK_PROT_SVR_H__
#define __INCLUDED_SOCK_PROT_SVR_H__
#include "Infra/Thread.h"
#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SockAcceptor.h"
#include "NetFramework/SockStream.h"
using namespace Dahua::NetFramework;

class CSockProtOther: public CNetHandler
{
public:
	CSockProtOther();
	~CSockProtOther();
	int RegisterSk( CSock& socket, SOCK_MASK type );
	int UnregisterSk(CSock& socket, SOCK_MASK type);
	int RemoveSk(CSock& socket);
public:
	virtual int handle_input( int handle );
};

class CSockProtSvr : public CNetHandler
{
public:
	CSockProtSvr();
	virtual ~CSockProtSvr();
public:
	int StartSvr( CSockAddrIPv4 & addr );
public:
	//注意，不同的套接字，或定时器的回调函数都有可能被同时回调，要作好共享数据的互斥访问。
	virtual int handle_input( int handle );
	virtual int handle_input_timeout( int handle );
	
	virtual int handle_close(CNetHandler *myself);
	
	void ThreadProc( Dahua::Infra::CThreadLite& arg );
private:
	//接收从m_acceptor中侦听到的套接字，要用指针，且要负责内存的释放。
	CSockStream*	m_stream;
	CSockAcceptor	m_acceptor;

	Dahua::Infra::CThreadLite* m_thread;
	CSockProtOther *m_spother;
};

int StartSockProtSvr();

#endif //__INCLUDED_SOCK_PROT_SVR_H__
