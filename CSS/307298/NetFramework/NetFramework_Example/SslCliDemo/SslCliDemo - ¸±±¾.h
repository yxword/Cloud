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
#ifndef __INCLUDED_SSL_CLI_DEMO_H__
#define __INCLUDED_SSL_CLI_DEMO_H__

#include "Infra/Mutex.h"
#include "Infra/Semaphore.h"
#include "Infra/PrintLog.h"

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SslStream.h"
#include "NetFramework/SslAsyncStream.h"

using namespace Dahua::NetFramework;
using namespace Dahua::Infra;

#ifdef _USE_OPENSSL

class CSslCliDemo : public CNetHandler
{
public:
	enum MsgCtxType
	{
		MsgCtxTypeTimeStampe,
		MsgCtxTypeHttp,
		MsgCtxTypeRawData
	};

	enum CliDemoMode
	{
		MODE_SYNC_IO,
		MODE_ASYNC_IO	
	};
public:
	CSslCliDemo();
	virtual ~CSslCliDemo();
public:
	int StartSession( CSockAddr & addr, bool asyncMode=false );
	int ReStartSession();
	void SetDebugFlag(bool flag);
	void SetBenchmkFlag(bool flag);
public:
	virtual int handle_output( int handle );
	virtual int handle_output_timeout( int handle );
	
	virtual int handle_input( int handle );
	//要检测socket上的异常，要实现该函数。
	virtual int handle_exception( int handle );
	
	virtual int handle_close(CNetHandler *myself);
public:
	bool isDebug();
	void  setExitFlag(bool flag);
	bool looping();
	void setSchedFlag(bool flag);
	bool isScheduled();
	bool isBenchmkTest();
private:
	int  sendMsg(MsgCtxType sctype);
	void setMode(bool asynflag);
	CSock *getSockBase();
private:
	CSslAsyncStream	m_async;
	CSslStream      m_stream;
	
	CliDemoMode 	m_mode;
	bool 			m_exitflag;
	bool 			m_scheduled;
	Dahua::Infra::CMutex m_mutex;
	bool			m_debug;

	// 异步SSL握手需要在handle_output中才能判断握手是否成功
	bool 			m_benchmarkTest;
};

struct DemoTestConfig
{
	bool help;
	bool asyncMode;	//  async io mode?
	bool debug;		// -d option, enable debug 
	bool benchmark;  // enable 
	int  interval;
	int parallel;		//  -P option, num of parallel channel
	int repeatCnt;
	int port;		// -p option, socket port 
	bool qps;		// queries 
	struct sockaddr_in addr; // -c host	

	DemoTestConfig():help(false),asyncMode(false), debug(false), benchmark(false), interval(0), parallel(1), repeatCnt(1), port(0), qps(false) 
	{
	}
};

// async ssl connection benchmark test 
struct ASyncConnBmtCost
{
	int onceInstNum;
	CSslCliDemo **onceClis;
	Dahua::Infra::CMutex mutex;	
	std::list<int> hdshakeQueue;	
	unsigned long tmCost;
	int 		  setupInst;
	bool          testDone;	
	ASyncConnBmtCost():onceInstNum(0), onceClis(NULL), tmCost(0), setupInst(0), testDone(false)
	{}
};

class HashList
{
public:
	struct Node
	{
		int handle;
		Node *next;
		Node():handle(-1), next(NULL)
		{}
	};
public:
	HashList(int hashsize = 16);
	~HashList();
	void Insert(int handle);
	void Delete(int handle);
private:
	int hashIndex(int handle);
private:
	Node 	**m_hashlist;
	int   	  m_hashsize;
	Dahua::Infra::CMutex m_mutex;	
	int	      m_instCnt;
};

#if 0
HashList::HashList(int hashsize)
{
	m_hashsize = hashsize;
	m_instCnt = 0;
	m_hashlist = new Node*[m_hashsize];
}

HashList::~HashList()
{
	HashList::Node *pInst, *pNode = NULL;

	m_mutex.enter();
	for (int i = 0; i < m_hashsize; i++)
	{
		pInst = m_hashlist[i];

		while (pInst)
		{
			pNode = pInst;
			pInst = pInst->next;
			delete pNode,pNode=NULL;
		}
	}
	m_mutex.leave();
	
	if (m_hashsize > 0)
		delete []m_hashlist, m_hashlist=NULL;
}

int HashList::hashIndex(int handle)
{
	return handle%m_hashsize;
}

void HashList::Insert(int handle)
{
	Node *pInst = NULL;
	Node **ppit = &m_hashlist[hashIndex(handle)];
	
	pInst = new Node;
	pInst->handle = handle;
	
	m_mutex.enter();
	while (*ppit)
	{
		ppit = &((*ppit)->next);
	}
	*ppit = pInst;
	m_instCnt++;
	m_mutex.leave();
}

void HashList::Delete(int handle)
{
	HashList::Node *pInst = NULL;
	HashList::Node **ppit = &m_hashlist[hashIndex(handle)];
	
	m_mutex.enter();
	while (*ppit)
	{
		if ((*ppit)->handle == handle)
		{
			pInst = *ppit;
			m_instCnt--;
			*ppit = pInst->next;
			break;
		}
		ppit = &((*ppit)->next);
	}
	m_mutex.leave();
}
//static HashList connHashList;

#endif 

class CCliDemoTest
{
public:
	CCliDemoTest();
	~CCliDemoTest();
public:
	int  doNormalTest(DemoTestConfig &dtconfig);
	void normalTestExit(DemoTestConfig & dtconfig);
	int  doBenchmarkTest(int argc, char * * argv, DemoTestConfig & dtconfig);
	void benchmarkTestExit(DemoTestConfig & dtconfig);
	int  doQpsTest(DemoTestConfig & dtconfig);
	void qpsTestExit(DemoTestConfig & dtconfig);
private:
	void adjustTxIntvl(DemoTestConfig &dtconfig, struct timeval &tv);
	int  doOnceTestSync(DemoTestConfig &config, unsigned long &tmCost);
	int  doBenchmarkTestAsync(int argc, char **argv, DemoTestConfig &dtconfig);
	int  doBenchmarkTestSync(int argc, char **argv, DemoTestConfig &dtconfig);
	int  doAsyncOnceTest(int idx, DemoTestConfig &config, ASyncConnBmtCost *pbmt);
	void asyncOnceTestCleanup(ASyncConnBmtCost *pbmt);
	int  doSyncOnceTest(DemoTestConfig &config, unsigned long &tmCost);
	int  doSyncQpsTest(DemoTestConfig &config);
	int  doAsyncQpsTest(DemoTestConfig &config);
	void qpsTestProc(unsigned long param);
	
private:
	CSslCliDemo *m_cliDemo; 
	int			 m_numInst;
};

int StartSslCliDemo(int argc, char **argv);

#endif

#endif //__INCLUDED_SSL_CLI_DEMO_H__
