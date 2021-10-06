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
#ifndef __INCLUDED_SOCK_PROTECT_CLIENT_H__
#define __INCLUDED_SOCK_PROTECT_CLIENT_H__
#include <time.h>
#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SockStream.h"
using namespace Dahua::NetFramework;
class CSockProtCli : public CNetHandler
{
public:
	CSockProtCli();
	virtual ~CSockProtCli();
public:
	int StartSession( CSockAddr & addr, int initial);
	virtual int handle_output( int handle );
	virtual int handle_output_timeout( int handle );
	//要检测socket上的异常，要实现该函数。
	virtual int handle_exception( int handle );
	
	virtual int handle_close(CNetHandler *myself);
	
private:
	int do_send();
private:
	CSockStream	m_stream;
};

int StartSockProtCli();

#endif //__INCLUDED_SOCK_PROTECT_CLIENT_H__