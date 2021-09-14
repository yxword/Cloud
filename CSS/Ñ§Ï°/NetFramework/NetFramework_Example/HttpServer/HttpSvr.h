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
	//ע�⣬��ͬ���׽��֣���ʱ���Ļص��������п��ܱ�ͬʱ�ص���Ҫ���ù������ݵĻ�����ʡ�
	virtual int handle_input( int handle );
	virtual int handle_input_timeout( int handle );
	virtual int handle_close(CNetHandler *myself);
private:
	int procConnRequest(int handle);
	int doHttpReqProc(CSockStream *stream, char *reqBufmsg);
	int httpRequestParser(char *reqBufmsg, HttpReqeust &request);
private:
	//���մ�m_acceptor�����������׽��֣�Ҫ��ָ�룬��Ҫ�����ڴ���ͷš�
	//���������׽��ֱ����κ�����Ż�, ģ������ҵ��ʹ�÷�ʽ

	Dahua::Infra::CMutex m_mutex;
	ConnSockTable	m_tables;
	CSockAcceptor	m_acceptor;
};

int StartHttpSvr(int argc, char **argv);

#endif //__INCLUDED_HTTP_SVR_H__
