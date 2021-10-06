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
#ifndef __INCLUDED_DHTS_CLIENT_PERFTEST_H__
#define __INCLUDED_DHTS_CLIENT_PERFTEST_H__

#include <map>
#include <list>
#include "Infra/Mutex.h"
#include "Infra/Semaphore.h"

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/DhtsStream.h"

#include "Dhts/Dhts.h"
#include "dhts_perftest.h"

using namespace Dahua::NetFramework;
using namespace Dahua::Infra;

struct ClientSession
{
	CDhtsStream stream;
	char *bufptr;
	int   bufsize;
	bool  seqtime_test;
	int   total_txbytes;
};

class CDhtsCliPerfTest : public CNetHandler
{
public:
	CDhtsCliPerfTest();
	~CDhtsCliPerfTest();
public:
	void SetPtconfig(PerfTestConfig &ptconfig);
	int StartSession();
	int RestartSession();
	void Close();
private:
	int calcTxbufsize();
	int setupClients();
	void destroyClients();
	int initSession(ClientSession *session, bool seqtime_test=false);
	void deinitSession(ClientSession *session);
	int processSession(int sockfd, int revents);
	int getSeqTime(char *bufptr, int len);
	int doSend(ClientSession *session);
	
private:
    virtual int handle_input( int handle );
	virtual int handle_output( int handle );
	virtual int handle_output_timeout( int handle );
	//要检测socket上的异常，要实现该函数。
	virtual int handle_exception( int handle );
	virtual int handle_input_timeout( int handle );
	virtual int handle_close(CNetHandler* myself);
private:
	typedef std::map<int,ClientSession*>::iterator SessionListIt;

	Dahua::Infra::CMutex	  	 m_mutex;
	std::map<int,ClientSession*> m_sessionlist;
	
	PerfTestConfig  m_ptconfig;
	bool 			m_inited;
	CSemaphore		m_sema;
};


#endif //__INCLUDED_DHTS_CLIENT_H__
