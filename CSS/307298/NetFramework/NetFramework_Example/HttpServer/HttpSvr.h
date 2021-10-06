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

#ifndef __INCLUDED_HTTP_SVR_H__
#define __INCLUDED_HTTP_SVR_H__

#include <map>
#include "Infra/Mutex.h"

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SockAcceptor.h"
#include "NetFramework/SockStream.h"

using namespace Dahua::NetFramework;

enum HttpReqMethod
{
	REQ_METHOD_GET,
	REQ_METHOD_POST
};

struct HttpReqeust
{
	HttpReqMethod method;
	char urlpath[32];
	char filename[32];
	HttpReqeust()
	{
		method = REQ_METHOD_GET;
		memset(urlpath, 0, sizeof(urlpath));
		memset(filename, 0, sizeof(filename));
	}
};

class CHttpSvr : public CNetHandler
{
public:
	typedef std::map<int, CSockStream*> ConnSockTable;
	typedef std::map<int, CSockStream*>::iterator ConnSockTableIt;
public:
	CHttpSvr();
	virtual ~CHttpSvr();
public:
	int StartSvr( CSockAddrIPv4 & addr );
public:
	//注意，不同的套接字，或定时器的回调函数都有可能被同时回调，要作好共享数据的互斥访问。
	virtual int handle_input( int handle );
	virtual int handle_input_timeout( int handle );
	virtual int handle_close(CNetHandler *myself);
private:
	int procConnRequest(int handle);
	int doHttpReqProc(CSockStream *stream, char *reqBufmsg);
	int httpRequestParser(char *reqBufmsg, HttpReqeust &request);
private:
	//接收从m_acceptor中侦听到的套接字，要用指针，且要负责内存的释放。
	//不对连接套接字表做任何设计优化, 模拟最常规的业务使用方式

	Dahua::Infra::CMutex m_mutex;
	ConnSockTable	m_tables;
	CSockAcceptor	m_acceptor;
};

int StartHttpSvr(int argc, char **argv);

#endif //__INCLUDED_HTTP_SVR_H__
