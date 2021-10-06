#include "SslX509.h"
#include "SslOption.h"
#include "NbSslSvr.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"

#ifdef _USE_OPENSSL

CNbSslSvr::CNbSslSvr()
{
}

CNbSslSvr::~CNbSslSvr()
{
	CSslAsyncStream *pnbstream = NULL;
	std::map<int, CSslAsyncStream *>::iterator pit;

	m_mutex.enter();

	for (pit = m_list.begin(); pit != m_list.end();)
	{
		pnbstream = pit->second;
		m_list.erase(pit++);
		if (pnbstream)
		{
			RemoveSock(*pnbstream);
			delete pnbstream, pnbstream=NULL;
		}
	}
	m_mutex.leave();

	RemoveSock(m_acceptor);
	m_acceptor.Close();
}

int CNbSslSvr::StartSvr( CSockAddrIPv4 & addr, int session )
{
  //设置密钥文件的路径
	char* path = "./../ssl_pem";
	//m_acceptor.SetPemPath( path );
	CSslX509* x509 = new CSslX509;
	if( x509->SetCert("./cert/EndProduct.crt", "./cert/EndProduct.key") ){
		delete x509, x509=NULL;
		return -1;
	}else
		printf("set certificate succeed!\n");
		
	//x509->SetCA("./cert/AffiliatedCA.crt");
	x509->SetCA("./cert/RootCA.crt");
	
	x509->SetTrustCA("./cert/RootCA.crt");
	
	m_acceptor.SetOption( Module_X509, Type_X509_input, x509 );
	if( session & Type_Session_id ){
		m_acceptor.SetOption( Module_Session, Type_Session_id, NULL );
		printf("set Session id succeed!\n");
	}else if( session & Type_Session_ticket ){
		m_acceptor.SetOption( Module_Session, Type_Session_ticket, NULL );
		printf("set Session ticket succeed!\n");
	}
		
		//建立侦听队列，出错的判断：返回值小于0 
	if( m_acceptor.Open( addr ) < 0 ){
		fprintf( stderr, "建立侦听套接字失败！\n" );
		return -1;
	}
	//注册读事件，当有客户端连接请求到达时，会回调该套接字的读回调函数handle_input。
	//该接口不必判断返回值是否成功。
	RegisterSock( m_acceptor, READ_MASK );
	return 0;
}

int CNbSslSvr::processConnRequest(int handle)
{
	int len = 0;
	char strmbuf[1024];
	CSslAsyncStream *pnbstream = NULL;
	std::map<int, CSslAsyncStream *>::iterator pit;
	
	m_mutex.enter();
	pit = m_list.find(handle);
	if (pit == m_list.end())
	{
		m_mutex.leave();
		return 0;
	}
	if ((pnbstream = pit->second) != NULL)
	{
		len = pnbstream->Recv(strmbuf, sizeof(strmbuf));
		if (len < 0)
		{
			m_list.erase(pit);
			m_mutex.leave();
			printf("remove connect session handle %d\n", pnbstream->GetHandle());
			RemoveSock(*pnbstream);
			delete pnbstream, pnbstream=NULL;
			return -1;
		}
		
		printf("<%d>: %s\n", handle, strmbuf);
	}
	
	m_mutex.leave();

	return 5000000;
}

int CNbSslSvr::handle_input( int handle )
{
	if( handle == m_acceptor.GetHandle() ){		//侦听队列上有连接请求到达。
		char ip_str[256];
		CSockAddrIPv4 remote_addr;
		CSslAsyncStream *pnbstream;
		pnbstream = m_acceptor.Accept( &remote_addr );
		if( pnbstream != NULL ){			//Accept成功的判断依据是指针不为NULL。		
			fprintf( stdout, "recv a new connection! form: %s:%d\n", 
					remote_addr.GetIpStr(ip_str, sizeof(ip_str)), remote_addr.GetPort() );
					
			m_mutex.enter();
			m_list[pnbstream->GetHandle()] = pnbstream;
			m_mutex.leave();

			RegisterSock( *pnbstream, READ_MASK, 5000000 );
			return 0;
		}
		return 0;
	}else 
	{
		return processConnRequest(handle);
	}
}

int CNbSslSvr::handle_input_timeout( int handle )
{
	CSslAsyncStream *pnbstream;
	
	printf("do handle %d handle_input_timeout\n", handle);
	
	m_mutex.enter();
	std::map<int, CSslAsyncStream *>::iterator pit = m_list.find(handle);
	if (pit != m_list.end())
	{
		pnbstream = pit->second;

		if (pnbstream)
		{
			RemoveSock( *pnbstream);
			m_list.erase(pit);
			delete pnbstream, pnbstream=NULL;
		}
	}
	m_mutex.leave();
	return 0;
}

int CNbSslSvr::handle_close(CNetHandler *myself)
{
	delete myself;
	return 0;
}

#if 0
#endif 

int StartNbSslSvr(int argc, char **argv)
{
	int session = 0;
	if( argc >= 2 )
		session = atoi(*(argv+1));

	CNbSslSvr *sslsvr = new CNbSslSvr;		
	CSockAddrIPv4 addr( INADDR_ANY, 9999 ); 	
	if( sslsvr->StartSvr( addr, 0) < 0 )
	{	
		printf("start ssl svr failed\n");
		return -1;
	}

    printf("start ssl svr listening on 9999 port\n");
	
	// 如果输入q 字符退出
	char c = 0;
	while((c = getchar()))
	{
		if ('q' == c)
		{
			printf("exit non-blocking ssl server\n");
			break;
		}

		Dahua::Infra::CThread::sleep(1 * 1000);
	}
	
	if (sslsvr)
		sslsvr->Close();
		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}

#endif
