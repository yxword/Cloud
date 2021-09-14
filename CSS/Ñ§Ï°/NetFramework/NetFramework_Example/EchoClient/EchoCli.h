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
#ifndef __INCLUDED_ECHO_SSL_CLIENT_H__
#define __INCLUDED_ECHO_SSL_CLIENT_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SockStream.h"

#ifdef _USE_OPENSSL
#include "NetFramework/SslStream.h"
#include "NetFramework/SslDgram.h"
#endif 

using namespace Dahua::NetFramework;

#ifdef _USE_OPENSSL
//#undef _USE_OPENSSL
#endif 

class CEchoCli : public CNetHandler
{
public:
	CEchoCli();
	virtual ~CEchoCli();
public:
	int StartSession( CSockAddr & addr );
	virtual int handle_output( int handle );
	virtual int handle_output_timeout( int handle );
	
	virtual int handle_input( int handle );
	//要检测socket上的异常，要实现该函数。
	virtual int handle_exception( int handle );
	
	virtual int handle_close(CNetHandler *myself);
private:
	int do_send();
private:
#ifdef _USE_OPENSSL
	CSslStream	m_stream;
#else
	CSockStream m_stream;
#endif
};

int StartEchoCli(int argc, char **argv);

#endif //__INCLUDED_ECHO_SSL_CLIENT_H__
