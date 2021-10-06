#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>

#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"
#include "SslX509.h"
#include "SslOption.h"
#include "SslSvrDemo.h"

#ifdef _USE_OPENSSL

CSslSvrDemo::CSslSvrDemo():m_mode(MODE_SYNC_IO), m_debug(false)
{
}

CSslSvrDemo::~CSslSvrDemo()
{
	if (isDebug())
	{
		CSock *sockptr = getSockBase();
		printf("~CSslSvrDemo, mode %s, handle %d\n", m_mode>0?"async":"sync", sockptr->GetHandle());
	}

	if (m_mode == MODE_ASYNC_IO)
	{
		destroyAsync();
	}
	else
	{
		destroySync();
	}
}

void CSslSvrDemo::destroySync()
{
	CSslStream *pstream = NULL;
	std::map<int, CSslStream *>::iterator pit;

	m_mutex.enter();

	for (pit = m_syncQueue.begin(); pit != m_syncQueue.end();)
	{
		pstream = pit->second;
		m_syncQueue.erase(pit++);
		if (pstream)
		{
			RemoveSock(*pstream);
			delete pstream, pstream=NULL;
		}
	}
	m_mutex.leave();

	RemoveSock(m_syncListener);
	m_syncListener.Close();

}

void CSslSvrDemo::destroyAsync()
{
	CSslAsyncStream *pnbstream = NULL;
	std::map<int, CSslAsyncStream *>::iterator pit;

	m_mutex.enter();

	for (pit = m_asyncQueue.begin(); pit != m_asyncQueue.end();)
	{
		pnbstream = pit->second;
		m_asyncQueue.erase(pit++);
		if (pnbstream)
		{
			RemoveSock(*pnbstream);
			delete pnbstream, pnbstream=NULL;
		}
	}
	m_mutex.leave();

	RemoveSock(m_asyncListener);
	m_asyncListener.Close();

}

int CSslSvrDemo::StartSvr( CSockAddrIPv4 & addr, bool asyncMode)
{
  	int session = 0;
	
  //设置密钥文件的路径
	char* path = "./../ssl_pem";
	//m_acceptor.SetPemPath( path );
	CSslX509* x509 = new CSslX509;
	if( x509->SetCert("./cert/EndProduct.crt", "./cert/EndProduct.key") )
	{
		Dahua::Infra::errorf_log("set certificate failed!\n");
		delete x509, x509=NULL;
		return -1;
	}else
		printf("set certificate succeed!\n");
		
	//x509->SetCA("./cert/AffiliatedCA.crt");
	x509->SetCA("./cert/RootCA.crt");
	
	x509->SetTrustCA("./cert/RootCA.crt");

	setMode(asyncMode);

	if (m_mode == MODE_ASYNC_IO)
	{		
		m_asyncListener.SetOption( Module_X509, Type_X509_input, x509 );
		if( session & Type_Session_id )
		{
			m_asyncListener.SetOption( Module_Session, Type_Session_id, NULL );
			printf("set Session id succeed!\n");
		}else if( session & Type_Session_ticket )
		{
			m_asyncListener.SetOption( Module_Session, Type_Session_ticket, NULL );
			printf("set Session ticket succeed!\n");
		}
			
		//建立侦听队列，出错的判断：返回值小于0 
		if( m_asyncListener.Open( addr ) < 0 )
		{
			Dahua::Infra::errorf_log("open async mode ssl linstener failed\n");
			return -1;
		}

		if (isDebug())
		{
			Dahua::Infra::warnf_log("start async ssl svr success, handle %d\n", m_asyncListener.GetHandle());
		}
	}
	else 
	{
		m_syncListener.SetOption( Module_X509, Type_X509_input, x509 );
		if( session & Type_Session_id )
		{
			m_syncListener.SetOption( Module_Session, Type_Session_id, NULL );
			printf("set Session id succeed!\n");
		}else if( session & Type_Session_ticket )
		{
			m_syncListener.SetOption( Module_Session, Type_Session_ticket, NULL );
			printf("set Session ticket succeed!\n");
		}
			
		//建立侦听队列，出错的判断：返回值小于0 
		if( m_syncListener.Open( addr ) < 0 )
		{
			Dahua::Infra::errorf_log("open sync mode ssl linstener failed\n");
			return -1;
		}
		
		if (isDebug())
		{
			Dahua::Infra::warnf_log("start sync ssl svr success, handle %d\n", m_syncListener.GetHandle());
		}

	}

	Dahua::Infra::warnf_log("start server on port %d success\n", addr.GetPort());

	CSock *baseptr = getSockBase();
	
	//注册读事件，当有客户端连接请求到达时，会回调该套接字的读回调函数handle_input。
	//该接口不必判断返回值是否成功。
	RegisterSock( *baseptr, READ_MASK );
	
	return 0;
}

int CSslSvrDemo::procConnMsgSync(int handle)
{
	int len = 0;
	char strmbuf[1500];
	CSslStream *pstream = NULL;
	std::map<int, CSslStream *>::iterator pit;
	
	m_mutex.enter();
	pit = m_syncQueue.find(handle);
	if (pit == m_syncQueue.end())
	{
		m_mutex.leave();
		return 0;
	}
	if ((pstream = pit->second) != NULL)
	{
		len = pstream->Recv(strmbuf, sizeof(strmbuf));
		if (len < 0)
		{
			m_syncQueue.erase(pit);
			m_mutex.leave();
			Dahua::Infra::warnf_log("remove connect session handle %d\n", pstream->GetHandle());
			RemoveSock(*pstream);
			delete pstream, pstream=NULL;
			return -1;
		}

		MsgCtxType msgType = (MsgCtxType)(strmbuf[0]);
		CSock *baseptr = getSockBase();
		if (msgType != MsgCtxTypeRawData)
		{
			printf("<%d>: %s\n", handle, (char*)(strmbuf+1));
		}
		else
		{
			printf("<%d>: rcv %d bytes raw data", baseptr->GetHandle(), len);
		}
	}
	
	m_mutex.leave();

	return 5000000;
}

int CSslSvrDemo::procConnMsgAsync(int handle)
{
	int len = 0;
	char strmbuf[1500];
	CSslAsyncStream *pnbstream = NULL;
	std::map<int, CSslAsyncStream *>::iterator pit;
	
	m_mutex.enter();
	pit = m_asyncQueue.find(handle);
	if (pit == m_asyncQueue.end())
	{
		m_mutex.leave();
		return 0;
	}
	
	if ((pnbstream = pit->second) != NULL)
	{
		len = pnbstream->Recv(strmbuf, sizeof(strmbuf));
		if (len < 0)
		{
			m_asyncQueue.erase(pit);
			m_mutex.leave();
			Dahua::Infra::warnf_log("remove connect session handle %d\n", pnbstream->GetHandle());
			RemoveSock(*pnbstream);
			delete pnbstream, pnbstream=NULL;
			return -1;
		}

		MsgCtxType msgType = (MsgCtxType)(strmbuf[0]);
		CSock *baseptr = getSockBase();
		if (msgType != MsgCtxTypeRawData)
		{
			printf("<%d>: %s\n", handle,  (char*)(strmbuf+1));
		}
		else
		{
			printf("<%d>: rcv %d bytes raw data", baseptr->GetHandle(), len);
		}
	}
	
	m_mutex.leave();

	return 5000000;
}

int CSslSvrDemo::procConnReqSync(int handle)
{
	if( handle == m_syncListener.GetHandle() )
	{ 	//侦听队列上有连接请求到达。
		char ip_str[256];
		CSockAddrIPv4 remote_addr;
		CSslStream *connStream;
		connStream = m_syncListener.Accept( &remote_addr );
		if( connStream != NULL )
		{	
			//Accept成功的判断依据是指针不为NULL。 	
			fprintf( stdout, "recv a new connection! form: %s:%d\n", 
					remote_addr.GetIpStr(ip_str, sizeof(ip_str)), remote_addr.GetPort() );
					
			m_mutex.enter();
			m_syncQueue[connStream->GetHandle()] = connStream;
			m_mutex.leave();

			RegisterSock( *connStream, READ_MASK, 5000000 );
			return 0;
		}
	}
	return -1;
}

int CSslSvrDemo::procConnReqAsync(int handle)
{
	if( handle == m_asyncListener.GetHandle() )
	{ 	//侦听队列上有连接请求到达。
		char ip_str[256];
		CSockAddrIPv4 remote_addr;
		CSslAsyncStream *pnbstream;
		pnbstream = m_asyncListener.Accept( &remote_addr );
		if( pnbstream != NULL )
		{	
			//Accept成功的判断依据是指针不为NULL。 	
			fprintf( stdout, "recv a new connection! form: %s:%d\n", 
					remote_addr.GetIpStr(ip_str, sizeof(ip_str)), remote_addr.GetPort() );
					
			m_mutex.enter();
			m_asyncQueue[pnbstream->GetHandle()] = pnbstream;
			m_mutex.leave();

			RegisterSock( *pnbstream, READ_MASK, 5000000 );
			return 0;
		}
	}

	return -1;
}

int CSslSvrDemo::handle_input( int handle )
{
	printf( "<%d>: do handle input process!\n", handle);

	CSock *baseptr = getSockBase();
	
	if( handle == baseptr->GetHandle())
	{		
		//侦听队列上有连接请求到达。
		if (m_mode == MODE_ASYNC_IO)
		{
			return procConnReqAsync(handle);
		}
		else
		{
			return procConnReqSync(handle);
		}
		
	}
	else 
	{
		if (m_mode == MODE_ASYNC_IO)
		{
			return procConnMsgAsync(handle);
		}
		else
		{
			return procConnMsgSync(handle);
		}
	}
}

int CSslSvrDemo::handle_input_timeout( int handle )
{
	printf( "<%d>: do handle input timeout process!\n", handle);

	CSock *basePtr = getSockBase();
 	if( handle == basePtr->GetHandle() )
	{
		RemoveSock( *basePtr );
		return -1;
	}

	if (m_mode == MODE_ASYNC_IO)
	{
		CSslAsyncStream *pnbstream;
		
		m_mutex.enter();
		std::map<int, CSslAsyncStream *>::iterator pit = m_asyncQueue.find(handle);
		if (pit != m_asyncQueue.end())
		{
			pnbstream = pit->second;

			if (pnbstream)
			{
				RemoveSock( *pnbstream);
				m_asyncQueue.erase(pit);
				delete pnbstream, pnbstream=NULL;
			}
		}
		m_mutex.leave();
	}
	else
	{
		CSslStream *pstream;
		
		m_mutex.enter();
		std::map<int, CSslStream *>::iterator pit = m_syncQueue.find(handle);
		if (pit != m_syncQueue.end())
		{
			pstream = pit->second;

			if (pstream)
			{
				RemoveSock( *pstream);
				m_syncQueue.erase(pit);
				delete pstream, pstream=NULL;
			}
		}
		m_mutex.leave();
	}
	
	return -1;
}

int CSslSvrDemo::handle_exception( int handle )
{	
	printf( "<%d>: do handle exception process!\n", handle);

	CSock *basePtr = getSockBase();
 	if( handle == basePtr->GetHandle() )
	{
		RemoveSock( *basePtr );
		return -1;
	}

	if (m_mode == MODE_ASYNC_IO)
	{
		CSslAsyncStream *pnbstream;
		
		m_mutex.enter();
		std::map<int, CSslAsyncStream *>::iterator pit = m_asyncQueue.find(handle);
		if (pit != m_asyncQueue.end())
		{
			pnbstream = pit->second;

			if (pnbstream)
			{
				RemoveSock( *pnbstream);
				m_asyncQueue.erase(pit);
				delete pnbstream, pnbstream=NULL;
			}
		}
		m_mutex.leave();
	}
	else
	{
		CSslStream *pstream;
		
		m_mutex.enter();
		std::map<int, CSslStream *>::iterator pit = m_syncQueue.find(handle);
		if (pit != m_syncQueue.end())
		{
			pstream = pit->second;

			if (pstream)
			{
				RemoveSock( *pstream);
				m_syncQueue.erase(pit);
				delete pstream, pstream=NULL;
			}
		}
		m_mutex.leave();
	}
	
	return -1;

	return -1;
}

int CSslSvrDemo::handle_close(CNetHandler *myself)
{
	delete myself;
	return 0;
}

CSock* CSslSvrDemo::getSockBase()
{
	if (m_mode == MODE_ASYNC_IO)
		return &m_asyncListener;
	else 
		return &m_syncListener;
}


void CSslSvrDemo::SetDebugFlag(bool flag)
{
	m_debug = flag;
}

void CSslSvrDemo::SetBenchmkFlag(bool flag)
{
	m_benchmarkTest = flag;
}

void CSslSvrDemo::setMode(bool asynflag)
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

bool CSslSvrDemo::isDebug()
{
	return m_debug;
}

bool CSslSvrDemo::isBenchmkTest()
{
	return m_benchmarkTest;
}


#if 0
#endif 

CSvrDemoTest::CSvrDemoTest():m_svrDemo(NULL)
{
}

CSvrDemoTest::~CSvrDemoTest()
{
}

int  CSvrDemoTest::doNormalTest(DemoTestConfig &dtconfig)
{
	m_svrDemo = new CSslSvrDemo;
	
	CSockAddrIPv4 addr( dtconfig.addr);
	
	m_svrDemo->SetDebugFlag(dtconfig.debug);

	m_svrDemo->SetBenchmkFlag(dtconfig.benchmark);
	
	if ( m_svrDemo->StartSvr( addr, dtconfig.asyncMode) < 0 )
	{
		Dahua::Infra::errorf_log("start ssl svr failed\n");
		return -1;
	}
	// 如果输入q 字符退出
	char c = 0;
	while((c = getchar()))
	{
		if ('q' == c)
		{
			Dahua::Infra::warnf_log("exit non-blocking ssl server\n");
			break;
		}

		Dahua::Infra::CThread::sleep(1 * 1000);
	}
	
	return 0;


}
void CSvrDemoTest::normalTestExit(DemoTestConfig & dtconfig)
{
	CSslSvrDemo *svrDemo = m_svrDemo; 
	
	if (svrDemo)
	{
		Dahua::Infra::warnf_log("close sever\n");
		svrDemo->Close();
		m_svrDemo = NULL;
	}		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	Dahua::Infra::warnf_log("normal test exit\n");

}
int  CSvrDemoTest::doBenchmarkTest(int argc, char * * argv, DemoTestConfig & dtconfig)
{
	doNormalTest(dtconfig);
		
	return 0;
}

void CSvrDemoTest::benchmarkTestExit(DemoTestConfig & dtconfig)
{
	normalTestExit(dtconfig);
}


#if 0
#endif 

static void svrDemoUsage()
{
	printf("--------------------------------------------------------------------------------------------\n");
	printf("-                                   Ssl Server Demo help                                   -\n");
	printf("--------------------------------------------------------------------------------------------\n");
	printf("- Usage: sslsvr [-s] [option]          Execute sslsvr program                              -\n");
	printf("-                                                                                          -\n");
	printf("- Client/Server:                                                                           -\n");
	printf("-    -a | --async                      asynchronize io mode, default is synchronize mode   -\n");
	printf("-    -d | --debug                      show more info                                      -\n");
	printf("-    -p | --port portno                listen port for server, or port to connect for cli  -\n");
	printf("-    -b | --benchmark                  do benchmark test for sync and async mode           -\n");
	printf("-    -h | --help                       show help info                                      -\n");
	printf("-                                                                                          -\n");
	printf("- Server specific:                                                                         -\n");
	printf("-    -s | --server                     run in server mode     							   -\n");
	printf("-                                                                                          -\n");
	printf("- Client specific:                                                                         -\n");
	printf("-    -c | --client                     run in client mode	 							   -\n");
	printf("-    -P | --parallel                   number of parallel connection, ranges is [1,10000]  -\n");
	printf("-    -r | --repeat                     times to do benchmark test, ranges is [1,1000]      -\n");
	printf("--------------------------------------------------------------------------------------------\n");
}

static int parseOptionArgs(int &argc, char *argv[], DemoTestConfig &dtconfig)
{
	int opt = -1;
	const char *shortOptStr = "adsp:br:h";
	int port, repeatCnt;  
	while (1) 
	{
		int optIdx = 0;
		static struct option longOpts[] = 
		{
			{"debug", no_argument, NULL, 'd'},
			{"async", no_argument, NULL, 'a'},
			{"server", no_argument, NULL, 's'},
			{"port", required_argument, NULL, 'p'},
			//{"parallel", required_argument, NULL, 'P'},
			//{"interval", required_argument, NULL, 'i'},
			{"repeat", required_argument, NULL, 'r'},
			{"benchmark", no_argument, NULL, 'b'},
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
			case 's':
				if (inet_pton(AF_INET, "0.0.0.0", (void*)&dtconfig.addr.sin_addr) < 0)
				{
					Dahua::Infra::errorf_log("invalid svr addr, see help, %s\n", strerror(errno));
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
			default:
				Dahua::Infra::errorf_log("invalid opt %c\n, see help", opt);
				return -1;
		}
	}

	dtconfig.addr.sin_port = htons(dtconfig.port);
	
	return 0;
}


static void printOptionValue(DemoTestConfig &cmdOption)
{
	char ip_str[256];

	CSockAddrIPv4 servaddr(cmdOption.addr);
		
	printf(" help %d, debug %d, asynMode %d, benchmark %d, parallel %d, host %s:%d\n",
			cmdOption.help, cmdOption.debug, cmdOption.asyncMode, cmdOption.benchmark, 
			cmdOption.parallel, servaddr.GetIpStr(ip_str, sizeof(ip_str)), servaddr.GetPort());
}

int StartSslSvrDemo(int argc, char **argv)
{
	DemoTestConfig config;
	
	if (argc < 3)
	{
		svrDemoUsage();
		return 0;
	}

	if (parseOptionArgs(argc, argv, config) < 0)
	{
		svrDemoUsage();
		return 0;
	}

	printOptionValue(config);

	if (config.help)
	{
		svrDemoUsage();
		return 0;
	}

	CSvrDemoTest demotest;

	if (config.benchmark)
	{
		demotest.doBenchmarkTest(argc, argv, config);
		
		demotest.benchmarkTestExit(config);
	}
	else
	{
		demotest.doNormalTest(config);
		
		demotest.normalTestExit(config);
	}
	
	return 0;
}


#endif
