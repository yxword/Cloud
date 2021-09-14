#ifdef _USE_OPENSSL
#include "SslX509.h"
#include "SslOption.h"
#endif
#include "EchoSvr.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <Infra/Guard.h>
#include "Infra/Thread.h"

using namespace Dahua::Infra;

CEchoSvr::CEchoSvr()
	//:m_stream(NULL)
{
}

CEchoSvr::~CEchoSvr()
{	
//	if( m_stream != NULL )
//		delete m_stream, m_stream = NULL;
	RemoveSock( m_acceptor );
	m_acceptor.Close();		//这是一个简单的服务器，只接收一次，就不侦听了。先注销再关掉。
}

int CEchoSvr::CertMode( int mode )
{
#ifdef _USE_OPENSS
	printf("SSL STREAM SVR!..........................\n");
	CSslX509* x509 = NULL;
	switch( mode ){
		case ECHO_SVR_CERT_NONE:
			break;
		case ECHO_SVR_CERT_SET:
		case ECHO_SVR_CERT_PEER:
			if( x509 = get_new_x509() ){
				if( mode == ECHO_SVR_CERT_SET )
					m_acceptor.SetOption( Module_X509, Type_X509_input, x509 );
				else
					m_acceptor.SetOption( Module_X509, Type_X509_all, x509 );
			}
			break;
		default:
			break;
	}
#else
	printf("NORMAL SVR!..........................\n");
#endif
	return 0;
}

#ifdef _USE_OPENSSL
CSslX509* CEchoSvr::get_new_x509()
{
	CSslX509* x509 = new CSslX509;
	if( x509->SetCert("./cert/EndProduct.crt", "./cert/EndProduct.key") 
		|| x509->SetCA("./cert/AffiliatedCA.crt")
		|| x509->SetCA("./cert/RootCA.crt")
		|| x509->SetTrustCA("./RootCA.crt") ){
		delete x509, x509=NULL;
		printf("set certificate failed!\n");
	}else
		printf("set certificate succeed!\n");
	return x509;
}
#endif

int CEchoSvr::StartSvr( CSockAddrIPv4 & addr )
{
  //设置密钥文件的路径
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

int CEchoSvr::handle_input( int handle )
{
	if( handle == m_acceptor.GetHandle() ){		//侦听队列上有连接请求到达。
		char ip_str[256];
		CSockAddrIPv4 remote_addr;
		//有链接请求时，取消已经注册的所有事件
		//UnregisterSock( m_acceptor, READ_MASK | WRITE_MASK );
		CCommonStream* stream = m_acceptor.Accept( &remote_addr );
		if( stream != NULL ){			//Accept成功的判断依据是指针不为NULL。		
			fprintf( stdout, "成功接受一个连接! 来自： %s:%d\n", 
					remote_addr.GetIpStr(ip_str, sizeof(ip_str)), remote_addr.GetPort() );
			//注册新的socket的读事件，准备接收客户端的信息。5秒超时。
			add_stream_to_store( stream );
			RegisterSock( *stream, READ_MASK, 5000000 );
			//RemoveSock( m_acceptor );
			//m_acceptor.Close();		//这是一个简单的服务器，只接收一次，就不侦听了。先注销再关掉。
			return 0;
		}else{
			fprintf(stdout," accept connection failed!\n");
		}
		return 0;
	}else{//if( m_stream != NULL && handle == m_stream->GetHandle() ){  //已连接上的客户端有数据到达。
		CCommonStream* stream = get_stream_from_store( handle );
		if( !stream ) return 0;
#define RECV_BUFFER_SIZE 1024*16+1
		char recv_buf[RECV_BUFFER_SIZE];
		memset( recv_buf, 0, sizeof(recv_buf) );
		int ret = stream->Recv( recv_buf, sizeof(recv_buf) );
		if( ret < 0 ){
			if( errno == ECONNRESET )
				fprintf( stderr, "link be closed!\n" );
			else
				fprintf( stderr, "recv error!\n" );
			remove_stream_from_store( stream->GetHandle() );
			//RemoveSock( *m_stream );
			//delete m_stream, m_stream = NULL;		//发生错误，清理。
			return 0;								//已经删除socket, 返回值已无意义。
		}else if( ret == 0 ){
			fprintf( stdout, "not recv any data, go on!\n" );
			return 0;	//返回0，表示还是以5秒超时等待客户端的数据。
		}else{
			//fprintf( stdout, "echo server recv data len: %d\n", ret );
			/*
			static unsigned long send_count = 0;
			int send_len = m_stream->Send( recv_buf, ret );
			if( send_len < 0 ){
				if( errno == ECONNRESET )
					fprintf( stderr, "link be closed!\n" );
				else
					fprintf( stderr, "send error!\n" );
				RemoveSock( *m_stream );
				delete m_stream, m_stream = NULL;
				return 0;	
			}else if( send_len < ret ){
				fprintf( stderr, "network is congestion, data is not send all [%d-%d=%d]!\n", ret, send_len, ret-send_len );	
			}else if( send_count++ % 10000 == 0 ){
				printf("EchoSvr do send succeed! %ld\n", send_count-1);
			}
			*/
			if( do_send( stream, recv_buf, ret ) < 0 )
				return 0;
			else
				return 3000000;		//返回，表示接下来以3秒超时等待客户端的数据。
		}
	}
	fprintf( stderr, "error,cann't run into here\n" );
	return 0;			
}

void CEchoSvr::add_stream_to_store( CCommonStream* stream )
{
	int fd = stream->GetHandle();
	printf(" add fd:%d  from m_store\n", fd );
	CGuard guard(m_mutex);
	std::map<int,CCommonStream*>::iterator it = m_store.find( fd );
	if( it != m_store.end() ){
		RemoveSock( *(it->second) );
		delete it->second;
		m_store.erase( it );
	}
	m_store[fd] = stream;
	// .insert( std::makepair<int,CSslStream*>(fd, stream) );
}

CCommonStream* CEchoSvr::get_stream_from_store( int fd )
{
	CGuard guard(m_mutex);
	std::map<int,CCommonStream*>::iterator it = m_store.find( fd );
	if( it != m_store.end() )
		return it->second;
	return NULL;
}

void CEchoSvr::remove_stream_from_store( int fd )
{
	printf(" remove fd:%d  from m_store\n", fd );
	CGuard guard(m_mutex);
	std::map<int,CCommonStream*>::iterator it = m_store.find( fd );
	if( it != m_store.end() ){
		RemoveSock( *(it->second) );
		delete it->second;
		m_store.erase( it );
	}
}

int CEchoSvr::handle_input_timeout( int handle )
{
	printf( "handle input timeout!\n" );
	remove_stream_from_store( handle );
	/*
	if( m_stream != NULL && handle == m_stream->GetHandle() ){
		fprintf( stderr, "wait for data timeout!\n" );
		RemoveSock( *m_stream );
		delete m_stream, m_stream = NULL;       //发生错误，清理。
		return 0;                               //已经删除socket, 返回值已无意义
	}
	*/
	fprintf( stderr, "错误，不可能运行到这儿!\n" );
	return 0;
}

int CEchoSvr::handle_exception( int handle )
{
	printf( "handle input timeout!\n" );
	remove_stream_from_store( handle );
	fprintf( stderr, "错误，不可能运行到这儿!\n" );
	return 0;
}

int CEchoSvr::do_send( CCommonStream* stream, char* recv_buf, int ret )
{
	static unsigned long send_count = 0;
	int send_len = stream->Send( recv_buf, ret );
	if( send_len < 0 ){
		if( errno == ECONNRESET )
			fprintf( stderr, "link be closed!\n" );
		else
			fprintf( stderr, "send error!\n" );
		remove_stream_from_store( stream->GetHandle() );
		//RemoveSock( *stream );
		//delete stream, stream = NULL;
		return -1;	
	}else if( send_len < ret ){
		fprintf( stderr, "network is congestion, data is not send all [%d-%d=%d]!\n", ret, send_len, ret-send_len );	
	}else if( send_count++ % 10000 == 0 ){
		printf("EchoSvr do send succeed! %ld\n", send_count-1);
	}
	return 0;
}


int CEchoSvr::handle_close(CNetHandler *myself)
{
	delete myself;
	return 0;
}

#if 0
#endif 

int StartEchoSvr()
{

	CEchoSvr *svr = new CEchoSvr;
	if( svr->CertMode( ECHO_SVR_CERT_SET ) < 0 )
		return -1;
	CSockAddrIPv4 addr( INADDR_ANY, 12345 );
	if( svr->StartSvr( addr ) < 0 )
		return -1;
	
	// 如果输入q 字符退出
	char ch = 0;
	while(ch = getchar())
	{
		if ('q' == ch)
		{
			printf("exit \n");
			break;
		}

		Dahua::Infra::CThread::sleep(1 * 1000);
	}
	
	if (svr)
		svr->Close();
		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}