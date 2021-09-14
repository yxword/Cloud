#include "SslX509.h"
#include "SslOption.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>

#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"
#include "Infra/Time.h"
#include "Infra/Timer.h"

#include "SslCliDemo.h"

using namespace Dahua::Infra;

#ifdef _USE_OPENSSL

static int wrIdx = 0;
static int queueSz = 0;
static ASyncConnBmtCost *tmCostQueue = NULL;

static std::map<int, ASyncConnBmtCost*> connHsQueue;
static Dahua::Infra::CMutex connHsmutex;	
static Dahua::Infra::CSemaphore sema;

static ASyncConnBmtCost *getCurrentBmt()
{
	if (tmCostQueue!=NULL)
		return &tmCostQueue[wrIdx];
	else
		return NULL;
}

static void addConnection(int handle)
{
	ASyncConnBmtCost *pbmt = getCurrentBmt();

	if (pbmt)
	{
		connHsmutex.enter();
		
		pbmt->mutex.enter();
		pbmt->hdshakeQueue.push_back(handle);
		pbmt->mutex.leave();

		connHsQueue[handle] = pbmt;
		connHsmutex.leave();
		//printf("add handle %d to bmt, has %u in list\n", handle, pbmt, pbmt->hdshakeQueue.size());
	}
}

static void delConnection(int handle, bool connHsOk)
{
	connHsmutex.enter();
	
	std::map<int, ASyncConnBmtCost*>::iterator mit = connHsQueue.find(handle);
	if (mit != connHsQueue.end())
	{
		ASyncConnBmtCost *pbmt = mit->second;
		if (pbmt)
		{
			std::list<int>::iterator it;
			pbmt->mutex.enter();
			for (it = pbmt->hdshakeQueue.begin(); it != pbmt->hdshakeQueue.end(); it++)
			{
				if (*it == handle)	
				{	
					//if (connHsOk)
						//pbmt->setupInst++;	// connection setup ok
					pbmt->hdshakeQueue.erase(it);
					break;
				}
			}

			if (pbmt->hdshakeQueue.empty())
			{
				if (connHsOk)
				{
					unsigned long doneTs = Dahua::Infra::CTime::getCurrentMicroSecond();
					//printf("pbmt %p test with ts (%lu, %lu), time cost %lu us\n", pbmt, pbmt->tmCost, doneTs, doneTs - pbmt->tmCost);
					pbmt->tmCost = doneTs - pbmt->tmCost;
					pbmt->testDone = true;	
				}
			}
			pbmt->mutex.leave();
		}

		connHsQueue.erase(mit);

		if (connHsQueue.empty())
		{
			if (connHsOk)
				Dahua::Infra::warnf_log("all conns has test finished\n");
			sema.post();
		}

		connHsmutex.leave();
	}
}

static bool qpsMode = false;
static std::list<int> connectingList;
static std::list<int> completeList;

static void connHsStart(int handle)
{
	connHsmutex.enter();
	connectingList.push_back(handle);
	connHsmutex.leave();
}

static void connHsDone(int handle)
{
	static std::list<int>::iterator it;

	connHsmutex.enter();
	for (it = connectingList.begin(); it != connectingList.end(); it++)
	{
		if (*it == handle)
		{
			connectingList.erase(it);
			completeList.push_back(handle);
			break;
		}
	}
	connHsmutex.leave();
}

static int getCompleteConnNum()
{
	return completeList.size();
}

CSslCliDemo::CSslCliDemo():m_mode(MODE_SYNC_IO), m_exitflag(false), m_scheduled(false), m_benchmarkTest(false)
{
}

CSslCliDemo::~CSslCliDemo()
{
	printf("~CSslCliDemo ...\n");
	if (m_mode == MODE_ASYNC_IO)
	{
		RemoveSock( m_async);
	}
	else
	{
		RemoveSock( m_stream );
	}
}

int CSslCliDemo::StartSession( CSockAddr & addr, bool asyncMode )
{
	int session = 0;

	//这个判断没有必要，Connect方法里会判断，写在这里是为了帮助更好地理解地址对象。
	if( addr.GetType() != CSockAddr::SOCKADDR_TYPE_IPV4 ){
		fprintf( stderr, "地址类型错误！\n");
		return -1;
	}
	//设置密钥文件的路径
	//char* path = "./../ssl_pem";
	//m_stream.SetPemPath( path );
	
	CSslX509* x509 = new CSslX509;
	if( x509->SetTrustCA("./cert/RootCA.crt") )
	{
		delete x509, x509=NULL;
		Dahua::Infra::errorf_log("set trust CA failed!\n");
		return -1;
	}else{
		//printf("set certificate succeed!\n");
		x509->SetCert("./cert/EndProduct.crt", "./cert/EndProduct.key");
		//x509->SetCA("./cert/AffiliatedCA.crt");
		x509->SetCA("./cert/RootCA.crt");
	}

	setMode(asyncMode);

	if (m_mode == MODE_ASYNC_IO)
	{
		Dahua::Infra::warnf_log("async client %p ssl connect start\n", this);
		
		m_async.SetOption( Module_X509, Type_X509_all, x509 );
		if( session & Type_Session_id )
		{
			m_async.SetOption( Module_Session, Type_Session_id, NULL );
		}
		else if( session & Type_Session_ticket )
		{
			m_async.SetOption( Module_Session, Type_Session_ticket, NULL );
		}

		if( m_async.Connect( addr ) < 0 )
		{
			Dahua::Infra::errorf_log( "ssl connect failed\n" );
			return -1;
		}

		if (isBenchmkTest())
		{
			if (qpsMode)
				connHsStart(m_async.GetHandle());
			else
				addConnection(m_async.GetHandle());
		}
	}
	else 
	{
		Dahua::Infra::warnf_log("sync ssl client %p connect start\n", this);
		
		m_stream.SetOption( Module_X509, Type_X509_all, x509 );
		if( session & Type_Session_id )
		{
			m_stream.SetOption( Module_Session, Type_Session_id, NULL );
		}
		else if( session & Type_Session_ticket )
		{
			m_stream.SetOption( Module_Session, Type_Session_ticket, NULL );
		}
		
		if( m_stream.Connect( addr ) < 0 )
		{
			Dahua::Infra::errorf_log( "ssl connect failed\n" );
			return -1;
		}

		if (isDebug())
			Dahua::Infra::warnf_log("sync ssl client %p connect succes, handle %d\n", this, m_stream.GetHandle());
	}
	
	CSock *sockptr = getSockBase();
	if (isDebug())
		Dahua::Infra::warnf_log("ssl client %p register handle %d write event\n", this, sockptr->GetHandle());

	printf("---- is benchmark test? %s; debug mode? %s\n", isBenchmkTest()?"yes":"no", isDebug()?"yes":"no");

	if (isBenchmkTest())
	{
		if (m_mode == MODE_ASYNC_IO)
		{
			RegisterSock( *sockptr, WRITE_MASK|READ_MASK, 5000000 );
		}
	}
	else 
	{
		//注册写事件
		RegisterSock( *sockptr, WRITE_MASK|READ_MASK, 5000000 );
	}
	
	return 0;
}

int CSslCliDemo::ReStartSession()
{
	CSock *sockptr = getSockBase();

	RegisterSock( *sockptr, WRITE_MASK|READ_MASK, 5000000 );

	return 0;
}

void CSslCliDemo::SetDebugFlag(bool flag)
{
	m_debug = flag;
}

void CSslCliDemo::SetBenchmkFlag(bool flag)
{
	m_benchmarkTest = flag;
}

int CSslCliDemo::handle_input( int handle )
{
	printf( "<%d>: do handle input process!\n", handle);

	CSock *sockptr = getSockBase();

	if (handle == sockptr->GetHandle())
	{
		int len;
		char strmbuf[1500];

		if (m_mode == MODE_ASYNC_IO)
		{
			len = m_async.Recv(strmbuf, sizeof(strmbuf));
		}
		else 
		{
			len = m_stream.Recv(strmbuf, sizeof(strmbuf));
		}
		if (len < 0)
		{
			Dahua::Infra::warnf_log("<%d>: socket error or peer close\n", sockptr->GetHandle());
			setExitFlag(true);			
			RemoveSock(*sockptr);
			return -1;
		}
		
		printf("<%d>: %s\n", handle, strmbuf);
	}
	
	return 0;
}

int CSslCliDemo::handle_output(int handle)
{
	static int loopcnt = 0;
	CSock *sockptr = getSockBase();
	
	//printf( "<%d>: do handle_output [%d] loops process\n", handle, ++loopcnt);

	if (handle == sockptr->GetHandle())
	{
		if (m_mode == MODE_ASYNC_IO)
		{
			if (m_async.GetConnectStatus() != CSslAsyncStream::SSL_CONNECTED)
			{
				Dahua::Infra::errorf_log("ssl connect failed\n");
				if (isBenchmkTest())
				{
					if (!qpsMode)
						delConnection(handle, false);
				}
				return -1;
			}
			if (isDebug())
				Dahua::Infra::warnf_log("async ssl client %p connect succes, handle %d\n", this, handle);

			if (isBenchmkTest())
			{
				if (qpsMode)
					connHsDone(handle);
				else
					delConnection(handle, true);
				UnregisterSock( *sockptr, WRITE_MASK ); 
				return 0;
			}
		}

		UnregisterSock( *sockptr, WRITE_MASK ); 

		sendMsg(MsgCtxTypeTimeStampe);
		return 0;
	}

	return -1;
}

int CSslCliDemo::handle_output_timeout( int handle )
{
	CSock *sockPtr = getSockBase();

	printf( "<%d>: do handle output timeout process!\n", handle);
	
	if( handle == sockPtr->GetHandle() ){
		printf( "link is timeout!\n" );
		if (isBenchmkTest())
		{
			if (!qpsMode)
				delConnection(handle, false);
		}
		RemoveSock( *sockPtr );
	}
	return -1;			//超时机制一律不再保留。
}

int CSslCliDemo::handle_exception( int handle )
{
	CSock *sockPtr = getSockBase();
	
	printf( "<%d>: do handle exception process!\n", handle);

	if( handle == sockPtr->GetHandle() ){
		printf( "connect exception!\n" );
		if (isBenchmkTest())
		{
			if (!qpsMode)
				delConnection(handle, false);
		}
		RemoveSock( *sockPtr );
	}
	return -1;
}


int CSslCliDemo::handle_close(CNetHandler *myself)
{
	delete myself;
	return 0;
}


void CSslCliDemo::setExitFlag(bool flag)
{
	m_mutex.enter();
	m_exitflag = flag;
	m_mutex.leave();
}

bool CSslCliDemo::looping()
{
	bool looping;
	
	m_mutex.enter();
	looping = !m_exitflag;
	m_mutex.leave();

	return looping;
}

bool CSslCliDemo::isBenchmkTest()
{
	return m_benchmarkTest;
}

void CSslCliDemo::setSchedFlag(bool flag)
{
	m_mutex.enter();
	m_scheduled = flag;
	m_mutex.leave();
}

bool CSslCliDemo::isScheduled()
{
	bool flag;
	
	m_mutex.enter();
	flag = m_scheduled;
	m_mutex.leave();
	return flag;	
}

bool CSslCliDemo::isDebug()
{
	return m_debug;
}

void CSslCliDemo::setMode(bool asynflag)
{
	if (asynflag)
	{
		m_mode = MODE_ASYNC_IO;
	}
	else
	{
		m_mode = MODE_SYNC_IO;
	}
}

CSock* CSslCliDemo::getSockBase()
{
	if (m_mode == MODE_ASYNC_IO)
		return &m_async;
	else 
		return &m_stream;
}


char https_str[]="GET / HTTP/1.1"
"Accept: application/x-ms-application, image/jpeg, application/xaml+xml, image/gif, image/pjpeg, application/x-ms-xbap, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/msword, */*"
"Accept-Language: zh-CN"
"User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; WOW64; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; .NET4.0C; .NET4.0E; InfoPath.2)"
"Accept-Encoding: gzip, deflate"
"If-Modified-Since: Thu, 17 Sep 2015 08:21:41 GMT"
"If-None-Match: \"1442478101:1e8d3\""
"Host: 172.29.2.171"
"Host: 172.29.2.171"
"Connection: Keep-Alive"
"Connection: Keep-Alive"
"Cookie: DHLangCookie30=English; DhWebCookie=%7B%22username%22%3A%22admin%22%2C%22pswd%22%3A%22%22%2C%22talktype%22%3A1%2C%22logintype%22%3A0%7D; pwdNoTip=1;e=768";

int  CSslCliDemo::sendMsg(MsgCtxType msgType)
{
	int  datalen = 0;
	char send_buf[1024];
	
	if (msgType == MsgCtxTypeHttp)
	{	
		send_buf[0] = MsgCtxTypeHttp;
		datalen += 1;
		strncpy( send_buf+datalen, https_str, sizeof(send_buf)-datalen );
		datalen += strlen(https_str) + 1;
	} 
	else if (msgType == MsgCtxTypeTimeStampe)
	{
		send_buf[0] = MsgCtxTypeTimeStampe;
		datalen += 1;
		time_t ticks = time(NULL);
		int cnt = snprintf(send_buf+datalen, sizeof(send_buf)-datalen-1, "%s\n", ctime(&ticks));
		send_buf[datalen+cnt] = 0;
		datalen += cnt + 1;
	}
	else if (msgType == MsgCtxTypeRawData)
	{
		send_buf[0] = MsgCtxTypeRawData;
		datalen = sizeof(send_buf);
	}
	else
	{
		printf("invalid send ctx type: %d\n", msgType);
		return -1;
	}

	int retval = 0;
	CSock *sockptr = getSockBase();
	if (m_mode == MODE_ASYNC_IO)
	{
		retval = m_async.Send( send_buf,  datalen);
	}
	else
	{
		retval = m_stream.Send(send_buf, datalen);
	}
	
	if( retval < 0 )
	{
		printf( "<%d>: send data error!\n", sockptr->GetHandle());
	}
	else
	{
		if( retval < datalen)
		{
			printf("<%d>: tx %d bytes, should be %d bytes\n", sockptr->GetHandle(), retval, datalen);
		}

		if (msgType == MsgCtxTypeTimeStampe || msgType == MsgCtxTypeHttp)
		{
			printf("<%d>: %s\n", sockptr->GetHandle(), (char *)(send_buf+1));
		}
		else 
		{
			printf("<%d>: tx %d bytes raw data\n", sockptr->GetHandle(), retval);
		}
	}
	return retval;
}

#if 0
#endif 

// sync io mode is correct, async mode is not correct
int CCliDemoTest::doSyncOnceTest(DemoTestConfig &config, unsigned long &tmCost)
{
	unsigned long tm_cost;
	int parallel = config.parallel;
	CSslCliDemo **sslCliArray = new CSslCliDemo*[parallel];

	for (int i = 0; i < parallel; i++)
		sslCliArray[i] = NULL;
	
	tm_cost = Dahua::Infra::CTime::getCurrentMicroSecond();
	
	for (int i = 0; i < parallel; i++)
	{
		sslCliArray[i] = new CSslCliDemo;
		sslCliArray[i]->SetBenchmkFlag(config.benchmark);
		sslCliArray[i]->SetDebugFlag(config.debug);
		CSockAddrIPv4 addr( config.addr);
		
		if ( sslCliArray[i]->StartSession( addr, false) < 0 )
		{
			printf("start session failed\n");
			return -1;
		}
	}

	tmCost = Dahua::Infra::CTime::getCurrentMicroSecond() - tm_cost;

	for (int i = 0; i <parallel; i++)
	{
		if (sslCliArray[i])
		{
			CSslCliDemo *demoTest = sslCliArray[i];
			demoTest->Close();
			
			if (i % 2 == 0)
			{				
				// 睡眠一会, 避免过多对象同时析构, 阻塞等待异步SSL内部handler异步handle_close析构而造成死锁
				Dahua::Infra::CThread::sleep( 50);
			}
			sslCliArray[i] = NULL;
		}
	}

	return 0;
}


int CCliDemoTest::doBenchmarkTestSync(int argc, char **argv, DemoTestConfig &dtconfig)
{
	int repeatCnt = 0;
	unsigned long totalTmCost = 0;

	repeatCnt = dtconfig.repeatCnt;

	for (int i = 0; i < repeatCnt; i++)
	{
		unsigned long onceTmCost = 0;
		if (doSyncOnceTest(dtconfig, onceTmCost) < 0)
		{	
			printf(" do the %d test failed\n", i);
			break;
		}

		printf("\trepeat no %dth of %d connection finished! cost %ld microseconds \n", i, dtconfig.parallel, onceTmCost);
		totalTmCost +=  onceTmCost;
	}

	printf("--------------------------------------------------------------------------------------------\n");
	printf("In general:\n");
	printf("\t%d times %d sync ssl connection setup ok, average cost %.2f \n",  repeatCnt, dtconfig.parallel, (double)(totalTmCost/repeatCnt));	
	printf("--------------------------------------------------------------------------------------------\n");

	return 0;
}

// async io mode is correct, async mode is not correct
int CCliDemoTest::doAsyncOnceTest(int idx, DemoTestConfig &config, ASyncConnBmtCost *pbmt)
{
	CSslCliDemo **sslCliArray = new CSslCliDemo*[pbmt->onceInstNum];

	for (int i = 0; i < pbmt->onceInstNum; i++)
		sslCliArray[i] = NULL;

	pbmt->tmCost = Dahua::Infra::CTime::getCurrentMicroSecond();
	pbmt->onceClis = sslCliArray;
	wrIdx = idx;

	printf("pbmt %p idx %d start ts %lu\n", pbmt, idx, pbmt->tmCost);
	
	for (int i = 0; i < pbmt->onceInstNum; i++)
	{
		sslCliArray[i] = new CSslCliDemo;
		sslCliArray[i]->SetBenchmkFlag(config.benchmark);
		sslCliArray[i]->SetDebugFlag(config.debug);
		CSockAddrIPv4 addr( config.addr);
		if ( sslCliArray[i]->StartSession( addr, true) < 0 )
		{
			printf("start session failed\n");
			return -1;
		}
	}

	return 0;
}

void CCliDemoTest::asyncOnceTestCleanup(ASyncConnBmtCost *pbmt)
{
	CSslCliDemo *pclis;

	for (int i = 0; i < pbmt->onceInstNum; i++)
	{
		pclis = pbmt->onceClis[i];
		if (pclis)
		{
			printf("asyn ssl client %p close\n", pclis);
			pclis->Close();
			pbmt->onceClis[i] = NULL;

			if (i % 2 == 0)
			{				
				// 睡眠一会, 避免过多对象同时析构, 阻塞等待异步SSL内部handler异步handle_close析构而造成死锁
				Dahua::Infra::CThread::sleep( 50);
			}
		}
	}	

	delete []pbmt->onceClis, pbmt->onceClis=NULL;

	pbmt->hdshakeQueue.clear();

}


int CCliDemoTest::doBenchmarkTestAsync(int argc, char **argv, DemoTestConfig &dtconfig)
{
	int repeatCnt = 0;
	unsigned long totalTmCost = 0;
	ASyncConnBmtCost *pbmt;

	repeatCnt = dtconfig.repeatCnt;

	pbmt = new ASyncConnBmtCost[repeatCnt];
	
	queueSz = repeatCnt;
	tmCostQueue = pbmt;

	for (int i = 0; i < repeatCnt; i++)
	{
		pbmt->onceInstNum = dtconfig.parallel;
		if (doAsyncOnceTest(i, dtconfig, pbmt) < 0)
		{	
			printf(" do the %d test failed, bmt %p\n", i, pbmt);
			break;
		}
		pbmt++;
	}

	// 超时等待测试完成
	if (sema.pend(30 * 1000) < 0)
	{
		printf("--- wait all connection done tiemout ---\n");
	}

	int sampleCnt = 0;
	ASyncConnBmtCost *pit = tmCostQueue;
	for (int i = 0; i < repeatCnt; i++)
	{
		if (pit->testDone)
		{
			printf("\t --- repeat no %dth of %d connections finished! cost %ld microseconds \n", i, dtconfig.parallel, pit->tmCost);
			totalTmCost += pit->tmCost;
			sampleCnt++;
		}
		pit++;
	}

	if (sampleCnt == 0)
	{
		printf("--------------------------------------------------------------------------------------------\n");
		printf("\tasync ssl connections handshake test not ready, please wait longer\n");
		printf("--------------------------------------------------------------------------------------------\n");
	}
	else 
	{
		printf("--------------------------------------------------------------------------------------------\n");
		printf("In general:\n");
		printf("\t%d times %d async ssl connection setup ok, average cost %.2f \n",  sampleCnt, dtconfig.parallel, (double)(totalTmCost/sampleCnt));	
		printf("--------------------------------------------------------------------------------------------\n");
	}
	

	return 0;
}


int CCliDemoTest::doBenchmarkTest(int argc, char **argv, DemoTestConfig &dtconfig)
{
	if (dtconfig.asyncMode)
	{
		printf("--------------------------------------------------------------------------------------------\n");
		printf("-                               Do Async Ssl Benchmark Test                                -\n");
		printf("--------------------------------------------------------------------------------------------\n");
		
		doBenchmarkTestAsync(argc, argv, dtconfig);
	}
	else
	{
		printf("--------------------------------------------------------------------------------------------\n");
		printf("-                               Do Sync Ssl Benchmark Test                                 -\n");
		printf("--------------------------------------------------------------------------------------------\n");
		
		doBenchmarkTestSync(argc, argv, dtconfig);
	}
	return 0;
}

void CCliDemoTest::benchmarkTestExit(DemoTestConfig &dtconfig)
{
	if (dtconfig.asyncMode)
	{
		Dahua::Infra::warnf_log("--- benchmark test exit begin ---\n");
		if (tmCostQueue)
		{
			ASyncConnBmtCost *pbmt = tmCostQueue;	
			for (int i = 0; i <queueSz; i++)
			{
				asyncOnceTestCleanup(pbmt);
				pbmt++;
			}

			delete []tmCostQueue, tmCostQueue=NULL;
			
			connHsQueue.clear();
		}	
		
	}
	Dahua::Infra::CThread::sleep(2 * 1000);

	Dahua::Infra::warnf_log("--- benchmark test exit end ---\n");

}

int CCliDemoTest::doQpsTest(DemoTestConfig &dtconfig)
{
	if (dtconfig.asyncMode)
	{
		printf("--------------------------------------------------------------------------------------------\n");
		printf("-                               Do Async Ssl Benchmark Qps Test                            -\n");
		printf("--------------------------------------------------------------------------------------------\n");
		
		doAsyncQpsTest(dtconfig);
	}
	else
	{
		printf("--------------------------------------------------------------------------------------------\n");
		printf("-                               Do Sync Ssl Benchmark Qps Test                             -\n");
		printf("--------------------------------------------------------------------------------------------\n");
		
		doSyncQpsTest(dtconfig);
	}
	return 0;
}


int CCliDemoTest::doAsyncQpsTest(DemoTestConfig &config)
{
	bool exitflag = false;
	CSslCliDemo* cliDemo =NULL;
	std::list<CSslCliDemo*> instList;
	std::list<CSslCliDemo*>::iterator it;

	Dahua::Infra::CTimer *timer = new Dahua::Infra::CTimer("qpsAsyncTest");
	timer->start(Dahua::Infra::CTimer::Proc(&CCliDemoTest::qpsTestProc, this), 1000, 0, (unsigned long)&exitflag);
	
	Dahua::Infra::warnf_log("--- qps test timer start --- \n");

	qpsMode = true;

	while (!exitflag)
	{
		cliDemo = new CSslCliDemo;
		cliDemo->SetBenchmkFlag(config.benchmark);
		cliDemo->SetDebugFlag(config.debug);
		CSockAddrIPv4 addr( config.addr);
		if ( cliDemo->StartSession( addr, true) < 0 )
		{
			printf("start session failed\n");
			cliDemo->Close();
			break;
		}

		instList.push_back(cliDemo);

		//Dahua::Infra::CThread::sleep( 2);
	}

	printf("--------------------------------------------------------------------------------------------\n");
	printf("In general:\n");
	printf("\tAsync ssl mode setup up %d connects per second, QPS %d\n", getCompleteConnNum(), getCompleteConnNum());	
	printf("--------------------------------------------------------------------------------------------\n");

	for (it = instList.begin(); it!=instList.end();)
	{
		cliDemo = *it;
		if (cliDemo)
		{
			cliDemo->Close();
			Dahua::Infra::CThread::sleep( 20);
		}
		instList.erase(it++);
	}
	timer->stopAndWait();
	delete timer, timer=NULL;
	return 0;
}

void CCliDemoTest::qpsTestProc(unsigned long param)
{
	*(bool*)param = true;
	Dahua::Infra::warnf_log("--- qps test timer proc --- \n");
}

int CCliDemoTest::doSyncQpsTest(DemoTestConfig &config)
{
	int setupConnsCnt = 0;
	bool exitflag = false;
	CSslCliDemo* cliDemo = NULL;
	std::list<CSslCliDemo*> instList;
	std::list<CSslCliDemo*>::iterator it;

	Dahua::Infra::CTimer *timer = new Dahua::Infra::CTimer("qpsSyncTest");
	timer->start(Dahua::Infra::CTimer::Proc(&CCliDemoTest::qpsTestProc, this), 1000, 0, (unsigned long)&exitflag);	

	uint64_t ts = Dahua::Infra::CTime::getCurrentMicroSecond();

	while (!exitflag)
	{
		cliDemo = new CSslCliDemo;
		cliDemo->SetBenchmkFlag(config.benchmark);
		cliDemo->SetDebugFlag(config.debug);
		
		CSockAddrIPv4 addr( config.addr);
		if ( cliDemo->StartSession( addr, false) < 0 )
		{
			printf("start session failed\n");
			cliDemo->Close();
			break;
		}

		setupConnsCnt++;
		instList.push_back(cliDemo);
	}

	ts = Dahua::Infra::CTime::getCurrentMicroSecond() - ts;

	printf("--------------------------------------------------------------------------------------------\n");
	printf("In general:\n");
	if (exitflag)
		printf("\tsync ssl mode setup up %d connects per second, QPS %d\n", setupConnsCnt, setupConnsCnt);	
	else
		printf("\tsync ssl mode setup up %d connects in %ld ms, QPS %ld\n", setupConnsCnt, ts/1000, setupConnsCnt * 1000000/ts);	
	printf("--------------------------------------------------------------------------------------------\n");

	for (it = instList.begin(); it!=instList.end();)
	{
		cliDemo = *it;
		if (cliDemo)
		{
			cliDemo->Close();
			Dahua::Infra::CThread::sleep( 20);
		}
		instList.erase(it++);
	}
	
	timer->stopAndWait();
	delete timer, timer=NULL;

	return 0;
}

void CCliDemoTest::qpsTestExit(DemoTestConfig &dtconfig)
{
	if (dtconfig.asyncMode)
	{
		Dahua::Infra::warnf_log("--- benchmark qps test exit begin ---\n");
		if (tmCostQueue)
		{
			ASyncConnBmtCost *pbmt = tmCostQueue;	
			for (int i = 0; i <queueSz; i++)
			{
				asyncOnceTestCleanup(pbmt);
				pbmt++;
			}

			delete []tmCostQueue, tmCostQueue=NULL;
			
			connHsQueue.clear();
		}	
		
	}
	Dahua::Infra::CThread::sleep(2 * 1000);

	Dahua::Infra::warnf_log("--- benchmark qps test exit end ---\n");

}


CCliDemoTest::CCliDemoTest():m_cliDemo(NULL),m_numInst(0)
{
}

CCliDemoTest::~CCliDemoTest()
{
}

void CCliDemoTest::normalTestExit(DemoTestConfig & dtconfig)
{
	CSslCliDemo *demoPtr = m_cliDemo;
	
	if (demoPtr)
	{
		Dahua::Infra::warnf_log("do normal exit\n");
		demoPtr->Close();
		
		m_cliDemo = NULL;
		m_numInst = 0;
	}

	Dahua::Infra::CThread::sleep(2 * 1000);
}

void CCliDemoTest::adjustTxIntvl(DemoTestConfig & dtconfig, struct timeval &tv)
{
	if (dtconfig.interval == 0)
	{
		tv.tv_sec = 0;
		tv.tv_usec = 10;
	}
	else
	{
		tv.tv_sec = dtconfig.interval;
		tv.tv_usec = 0;
	}
}

int CCliDemoTest::doNormalTest(DemoTestConfig &dtconfig)
{
	m_numInst = 1;
	m_cliDemo = new CSslCliDemo;

	m_cliDemo->SetDebugFlag(dtconfig.debug);
	
	CSockAddrIPv4 addr( dtconfig.addr);
	if ( m_cliDemo->StartSession( addr, dtconfig.asyncMode ) < 0 )
	{
		Dahua::Infra::errorf_log("sslcli start session failed\n");
		return -1;
	}
	
	fd_set	rdset;
	FD_ZERO(&rdset);
	FD_SET(0, &rdset);	// listen stdin 
	//fcntl(0, F_SETFL, O_NONBLOCK );
	struct timeval tv;

	while (m_cliDemo->looping())
	{
		adjustTxIntvl(dtconfig, tv);
		
		FD_SET(0, &rdset);
		int nSelect = select(1, &rdset, NULL, NULL, &tv);
		if( nSelect == -1 )
		{			
			if( errno == EINTR )
			{
				continue;
			}else
			{
				Dahua::Infra::errorf_log( "select failed, error:%d, %s\n", errno, strerror( errno ));
				return -1;
			}
		}
		else if( nSelect == 0 )	
		{
			m_cliDemo->ReStartSession();	
		}
		else
		{
			// 如果输入q 字符退出
			char ch = getchar(); getchar();
			printf("--------- usr input ch: %c ------\n", ch);
			if ((ch == 'q') || (ch == 'Q')) 
			{
				Dahua::Infra::warnf_log("exit cli test\n");
				break;	
			}	
		}
		
	}

	return 0;
}

#if 0
#endif 
/*
     Normal  Test Mode
   		Sync Client :   ./sslcli -c 127.0.0.1 -p 7070 -d -i 1 
   		Async Client: 	./sslcli -c 127.0.0.1 -p 7070 -d -i 1 -a
 */
static void cliDemoUsage()
{
	printf("--------------------------------------------------------------------------------------------\n");
	printf("-                              		Ssl Client Demo help                        	 	   -\n");
	printf("--------------------------------------------------------------------------------------------\n");
	printf("- Usage: sslcli [-c host] [option]   Execute sslcli program                   	       	   -\n");
	printf("-                                                                                          -\n");
	printf("- Client/Server:											                               -\n");
	printf("-    -a | --async                      asynchronize io mode, default is synchronize mode   -\n");
	printf("-    -d | --debug                      show more info                                      -\n");
	printf("-    -p | --port portno                listen port for server, or port to connect for cli  -\n");
	printf("-    -b | --benchmark                  do benchmark test for sync and async mode           -\n");
	printf("-    -h | --help                       show help info  							           -\n");
	printf("-                                                                                          -\n");
	printf("- Server specific:													                       -\n");
	printf("-    -s | --server                     run in server mode	 							   -\n");
	printf("-                                                                                          -\n");
	printf("- Client specific:													                       -\n");
	printf("-    -c | --client                     run in client mode	 							   -\n");
	printf("-    -P | --parallel                   number of parallel connection, ranges is [1,10000]  -\n");
	printf("-    -r | --repeat                     times to do benchmark test, ranges is [1,1000]      -\n");
	printf("-    -q | --qps                        number of queries per second     				   -\n");
	printf("--------------------------------------------------------------------------------------------\n");
}

static int parseOptionArgs(int &argc, char *argv[], DemoTestConfig &dtconfig)
{
	int opt = -1;
	const char *shortOptStr = "adc:p:P:bi:r:qh";
	int port, repeatCnt, parallel, intvl;  

	while (1) 
	{
		int optIdx = 0;
		static struct option longOpts[] = 
		{
			{"debug", no_argument, NULL, 'd'},
			{"async", no_argument, NULL, 'a'},
			{"client", required_argument, NULL, 'c'},
			{"port", required_argument, NULL, 'p'},
			{"parallel", required_argument, NULL, 'P'},
			{"interval", required_argument, NULL, 'i'},
			{"repeat", required_argument, NULL, 'r'},
			{"benchmark", no_argument, NULL, 'b'},
			{"qps", no_argument, NULL, 'q'},
			{"help", no_argument, NULL, 'h'},
			{NULL, 0, NULL, 0},
		};

		opt = getopt_long(argc, argv, shortOptStr, longOpts, &optIdx);		
		if (opt == -1)
		{
			break;
		}

		switch (opt)
		{
			case 'h':
				dtconfig.help = true;
				break;
			case 'a':
				dtconfig.asyncMode = true;
				break;
			case 'd':
				dtconfig.debug = true;
				break;
			case 'c':
				if (inet_pton(AF_INET, optarg, &dtconfig.addr.sin_addr) < 0)
				{
					Dahua::Infra::errorf_log("invalid client option args, see help, %s\n", strerror(errno));
					return -1;
				}
				break;
			case 'p':
				port = atoi(optarg);
				if (port == 0)
				{
					Dahua::Infra::errorf_log("invalid port option args, see help\n");
					return -1;
				}
				dtconfig.port = port;
				break;
			case 'P':
				parallel = atoi(optarg);
				if (parallel == 0)
				{
					Dahua::Infra::errorf_log("invalid port parallel args, see help\n");
					return -1;
				}
				dtconfig.parallel = parallel;
				break;
			case 'i':
				intvl = atoi(optarg);
				if (intvl <= 0)
					intvl = 1;			
				if (intvl > 5)
					intvl = 5;
				dtconfig.interval = intvl;
				break;
			case 'r':
				repeatCnt = atoi(optarg);
				if (repeatCnt <= 0)
					repeatCnt = 1;			
				if (repeatCnt > 1000)
					repeatCnt = 1000;
				dtconfig.repeatCnt = repeatCnt;
				break;
			case 'b':
				dtconfig.benchmark = true;
				break;
			case 'q':
				dtconfig.qps = true;
				break;
			default:
				Dahua::Infra::errorf_log("invalid opt %c\n, see sysWatch help\n", opt);
				return -1;
		}
	}

	if (dtconfig.parallel <= 1)
		dtconfig.parallel = 1;
	else if (dtconfig.parallel >= 10000)
		dtconfig.parallel = 10000;

	dtconfig.addr.sin_family = AF_INET;
	dtconfig.addr.sin_port = htons(dtconfig.port);
	
	return 0;
}


static void printOptionValue(DemoTestConfig &cmdOption)
{
	char ip_str[256];

	CSockAddrIPv4 servaddr(cmdOption.addr);
		
	printf(" help %d, debug %d, asynMode %d, benchmark %d, interval %d, parallel %d, repeatCnt %d, qps %d, host %s:%d\n",
			cmdOption.help, cmdOption.debug, cmdOption.asyncMode, cmdOption.benchmark, cmdOption.interval, 
			cmdOption.parallel, cmdOption.repeatCnt, cmdOption.qps, servaddr.GetIpStr(ip_str, sizeof(ip_str)), servaddr.GetPort());
}

int StartSslCliDemo(int argc, char **argv)
{
	DemoTestConfig config;
	
	if (argc < 3)
	{
		cliDemoUsage();
		return 0;
	}

	if (parseOptionArgs(argc, argv, config) < 0)
	{
		cliDemoUsage();
		return 0;
	}

	printOptionValue(config);

	if (config.help)
	{
		cliDemoUsage();
		return 0;
	}

	CCliDemoTest demotest;

	if (config.benchmark)
	{
		if (config.qps)
		{
			demotest.doQpsTest(config);
			demotest.qpsTestExit(config);
		}
		else 
		{
			demotest.doBenchmarkTest(argc, argv, config);
			demotest.benchmarkTestExit(config);
		}
	}
	else
	{
		demotest.doNormalTest(config);
		
		demotest.normalTestExit(config);
	}
	
	return 0;
}

#endif

