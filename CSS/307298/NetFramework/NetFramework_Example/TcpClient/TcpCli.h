//
//  "$Id: TcpCli.h 7672 2010-5-17 02:28:18Z he_linqiang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-5-17        he_linqiang Create
//
//
#ifndef __INCLUDED_TCP_CLIENT_H__
#define __INCLUDED_TCP_CLIENT_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SockStream.h"
using namespace Dahua::NetFramework;
class CTcpCli : public CNetHandler
{
public:
	CTcpCli();
	virtual ~CTcpCli();
public:
	int StartSession( CSockAddr & addr );
	virtual int handle_output( int handle );
	virtual int handle_output_timeout( int handle );
	virtual int handle_close(CNetHandler *myself);
	//要检测socket上的异常，要实现该函数。
	virtual int handle_exception( int handle );
private:
	int do_send();
private:
	CSockStream	m_stream;
};

int StartTcpCli();

#endif //__INCLUDED_TCP_CLIENT_H__
