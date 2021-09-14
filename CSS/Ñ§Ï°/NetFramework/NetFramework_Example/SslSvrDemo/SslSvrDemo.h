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

#ifndef __INCLUDED_SSL_SVR_DEMO_H__
#define __INCLUDED_SSL_SVR_DEMO_H__

#include <map>
#include <list>

#include "Infra/Mutex.h"
#include "Infra/Semaphore.h"
#include "Infra/PrintLog.h"

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SslAsyncAcceptor.h"
#include "NetFramework/SslAsyncStream.h"
#include "NetFramework/SslStream.h"
#include "NetFramework/SslAcceptor.h"


using namespace Dahua::Infra;
using namespace Dahua::NetFramework;

#ifdef _USE_OPENSSL

class CSslSvrDemo : public CNetHandler
{
public:
	enum MsgCtxType
	{
		MsgCtxTypeTimeStampe,
		MsgCtxTypeHttp,
		MsgCtxTypeRawData
	};

	enum SvrDemoMode
	{
		MODE_SYNC_IO,
		MODE_ASYNC_IO	
	};
		
public:
	CSslSvrDemo();
	virtual ~CSslSvrDemo();
public:
	int StartSvr( CSockAddrIPv4 & addr, bool asyncMode);
	void SetDebugFlag(bool flag);
	void SetBenchmkFlag(bool flag);
public:
	//注意，不同的套接字，或定时器的回调函数都有可能被同时回调，要作好共享数据的互斥访问。
	virtual int handle_input( int handle );
	virtual int handle_input_timeout( int handle );
	virtual int handle_exception( int handle );
	virtual int handle_close(CNetHandler *myself);
	
private:
	bool isDebug();
	bool isBenchmkTest();
	void setMode(bool asynflag);
	CSock *getSockBase();
	void destroySync();
	void destroyAsync();
	int procConnReqSync(int handle);
	int procConnReqAsync(int handle);
	int procConnMsgAsync(int handle);
	int procConnMsgSync(int handle);
private:
	//接收从m_acceptor中侦听到的套接字，要用指针，且要负责内存的释放。
	Dahua::Infra::CMutex	   m_mutex;
	
	std::map<int, CSslAsyncStream*> m_asyncQueue;
	std::map<int, CSslStream*> m_syncQueue;
	
	CSslAsyncAcceptor	m_asyncListener;
	CSslAcceptor        m_syncListener;
	
	SvrDemoMode			m_mode;
	bool		    	m_debug;
	bool 				m_benchmarkTest;
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
	struct sockaddr_in addr; // -c host	

	DemoTestConfig():help(false),asyncMode(false), debug(false), benchmark(false), interval(0), parallel(1), repeatCnt(1), port(0) 
	{
	}
};

class CSvrDemoTest
{
public:
	CSvrDemoTest();
	~CSvrDemoTest();
public:
	int  doNormalTest(DemoTestConfig &dtconfig);
	void normalTestExit(DemoTestConfig & dtconfig);
	int  doBenchmarkTest(int argc, char * * argv, DemoTestConfig & dtconfig);
	void benchmarkTestExit(DemoTestConfig & dtconfig);
private:
	CSslSvrDemo *m_svrDemo; 
};

int StartSslSvrDemo(int argc, char **argv);

#endif

#endif //__INCLUDED_SSL_SVR_DEMO_H__
