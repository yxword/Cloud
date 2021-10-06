#include "SslX509.h"
#include "SslSvr.h"
#include "SslOption.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"

#ifdef _USE_OPENSSL

CSslSvr::CSslSvr()
	:m_stream(NULL)
{
}

CSslSvr::~CSslSvr()
{
	if( m_stream != NULL )
		delete m_stream, m_stream = NULL;
}

int CSslSvr::StartSvr( CSockAddrIPv4 & addr, int session )
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

void CSslSvr::StopSvr()
{
	RemoveSock(m_acceptor);
}

int CSslSvr::handle_input( int handle )
{
	if( handle == m_acceptor.GetHandle() ){		//侦听队列上有连接请求到达。

		if( m_stream != NULL ){
			fprintf( stderr, "just surport only one client!\n" );
			return -1;
		}
		char ip_str[256];
		CSockAddrIPv4 remote_addr;
		//有链接请求时，取消已经注册的所有事件
		UnregisterSock( m_acceptor, READ_MASK | WRITE_MASK );
		m_stream = m_acceptor.Accept( &remote_addr );
		if( m_stream != NULL ){			//Accept成功的判断依据是指针不为NULL。		
			fprintf( stdout, "recv a new connection! form: %s:%d\n", 
					remote_addr.GetIpStr(ip_str, sizeof(ip_str)), remote_addr.GetPort() );
			#if 0
			//注册新的socket的读事件，准备接收客户端的信息。5秒超时。
			delete m_stream, m_stream = NULL;
			RegisterSock( m_acceptor, READ_MASK | WRITE_MASK );
			return 0;
			#endif 
			RegisterSock( *m_stream, READ_MASK, 50000000 );
			
			RegisterSock( m_acceptor, READ_MASK | WRITE_MASK );
			/* 
			RemoveSock( m_acceptor );
			m_acceptor.Close();		//这是一个简单的服务器，只接收一次，就不侦听了。先注销再关掉。
			*/
			return 0;
		}
		return 0;
	}else if( m_stream != NULL && handle == m_stream->GetHandle() ){  //已连接上的客户端有数据到达。
		char recv_buf[1024];
		memset( recv_buf, 0, sizeof(recv_buf) );
		int ret = m_stream->Recv( recv_buf, sizeof(recv_buf) );
		if( ret < 0 ){
			if( errno == ECONNRESET )
				fprintf( stderr, "link be closed!\n" );
			else
				fprintf( stderr, "recv error!\n" );
			RemoveSock( *m_stream );
			delete m_stream, m_stream = NULL;		//发生错误，清理。
			return 0;								//已经删除socket, 返回值已无意义。
		}else if( ret == 0 ){
			fprintf( stdout, "not recv any data, go on!\n" );
			return 0;	//返回0，表示还是以5秒超时等待客户端的数据。
		}else{
			fprintf( stdout, "recv data:\n%s\n", recv_buf );
			RemoveSock( *m_stream );				// 仅接受一次
			delete m_stream, m_stream = NULL;
			return 0;						
			return 3000000;		//返回，表示接下来以3秒超时等待客户端的数据。
		}
	}
	fprintf( stderr, "error,cann't run into here\n" );
	return 0;			
}

int CSslSvr::handle_input_timeout( int handle )
{
	printf( "handle input timeout!\n" );
	if( m_stream != NULL && handle == m_stream->GetHandle() ){
		fprintf( stderr, "wait for data timeout!\n" );
		RemoveSock( *m_stream );
		delete m_stream, m_stream = NULL;       //发生错误，清理。
		return 0;                               //已经删除socket, 返回值已无意义
	}
	fprintf( stderr, "错误，不可能运行到这儿!\n" );
	return 0;
}

int CSslSvr::handle_close(CNetHandler *myself)
{
	delete myself;
	return 0;
}

#if 0
#endif 

int StartSslSvr(int argc, char **argv)
{
	int session = 0;
	if( argc >= 2 )
		session = atoi(*(argv+1));

	CSslSvr *sslsvr = new CSslSvr;		
	CSockAddrIPv4 addr( INADDR_ANY, 9999 ); 	
	if( sslsvr->StartSvr( addr, 0) < 0 )
	{	
		printf("start ssl svr failed\n");
		return -1;
	}

    printf("start ssl svr listening on 9999 port\n");
	
	// 如果输入"quit" 字符退出
	char buf[32];
	while (fgets(buf, sizeof(buf), stdin))
	{
		if (!strncmp(buf, "quit", 4))
		{
			printf("exit \n");
			break;
		}

		Dahua::Infra::CThread::sleep(1 * 1000);
	}
	
	if (sslsvr)
	{
		sslsvr->StopSvr();
		sslsvr->Close();
	}	
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}

#endif
