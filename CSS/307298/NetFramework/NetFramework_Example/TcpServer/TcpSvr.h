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
	//注意，不同的套接字，或定时器的回调函数都有可能被同时回调，要作好共享数据的互斥访问。
	virtual int handle_input( int handle );
	virtual int handle_input_timeout( int handle );
	virtual int handle_close(CNetHandler *myself);
private:
	//接收从m_acceptor中侦听到的套接字，要用指针，且要负责内存的释放。
	CSockStream*	m_stream;
	CSockAcceptor	m_acceptor;
};

int StartTcpSvr();

#endif //__INCLUDED_TCP_SVR_H__
