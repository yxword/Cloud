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

#ifndef __INCLUDED_ECHO_SSL_SVR_H__
#define __INCLUDED_ECHO_SSL_SVR_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#ifdef _USE_OPENSSL
#include "NetFramework/SslAcceptor.h"
#include "NetFramework/SslStream.h"
#endif 
#include "NetFramework/SockAcceptor.h"
#include "Infra/Mutex.h"
#include <map>

using namespace Dahua::NetFramework;

#define ECHO_SVR_CERT_NONE 0 
#define ECHO_SVR_CERT_SET  1 
#define ECHO_SVR_CERT_PEER  2 

#ifdef _USE_OPENSSL
//#undef _USE_OPENSSL
#include "NetFramework/SslDgram.h"
#endif 

class CEchoSvr : public CNetHandler
{
public:
	CEchoSvr();
	virtual ~CEchoSvr();
public:
	int CertMode( int mode );
	int StartSvr( CSockAddrIPv4 & addr );
public:
	//注意，不同的套接字，或定时器的回调函数都有可能被同时回调，要作好共享数据的互斥访问。
	virtual int handle_input( int handle );
	virtual int handle_input_timeout( int handle );
	virtual int handle_exception( int handle );
	
	virtual int handle_close(CNetHandler *myself);
	
#ifdef _USE_OPENSSL
private:
	CSslX509* get_new_x509();
private:
	//接收从m_acceptor中侦听到的套接字，要用指针，且要负责内存的释放。
#define CCommonStream CSslStream
	std::map<int, CSslStream*> m_store;
	CSslStream*		m_stream;
	CSslAcceptor	m_acceptor;
#else
#define CCommonStream CSockStream
	std::map<int, CSockStream*> m_store;
	CSockStream* 	m_stream;
	CSockAcceptor	m_acceptor;
#endif
private:
	Dahua::Infra::CMutex m_mutex;
private:
	int do_send( CCommonStream* stream, char* recv_buf, int ret );
	void add_stream_to_store( CCommonStream* stream );
	CCommonStream* get_stream_from_store( int fd );
	void remove_stream_from_store( int fd );
};

int StartEchoSvr();

#endif //__INCLUDED_ECHO_SVR_H__
