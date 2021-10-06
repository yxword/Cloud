//
//  "$Id: SslCli.h 7672 2010-5-28 02:28:18Z zmw $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-5-28        zmw				 Create
//
//
#ifndef __INCLUDED_SSL_CLIENT_H__
#define __INCLUDED_SSL_CLIENT_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SslStream.h"
using namespace Dahua::NetFramework;

#ifdef _USE_OPENSSL

class CSslCli : public CNetHandler
{
public:
	CSslCli();
	virtual ~CSslCli();
public:
	int StartSession( CSockAddr & addr, int session );
	virtual int handle_output( int handle );
	virtual int handle_output_timeout( int handle );
	
	virtual int handle_input( int handle );
	//要检测socket上的异常，要实现该函数。
	virtual int handle_exception( int handle );
	
	virtual int handle_close(CNetHandler *myself);
private:
	int do_send();
private:
	CSslStream	m_stream;
};

int StartSslCli(int argc, char **argv);

#endif

#endif //__INCLUDED_SSL_CLIENT_H__
