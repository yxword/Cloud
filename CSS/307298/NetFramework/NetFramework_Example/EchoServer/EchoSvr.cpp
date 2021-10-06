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
	m_acceptor.Close();		//����һ���򵥵ķ�������ֻ����һ�Σ��Ͳ������ˡ���ע���ٹص���
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
  //������Կ�ļ���·��
	//�����������У�������жϣ�����ֵС��0 
	if( m_acceptor.Open( addr ) < 0 ){
		fprintf( stderr, "���������׽���ʧ�ܣ�\n" );
		return -1;
	}
	//ע����¼������пͻ����������󵽴�ʱ����ص����׽��ֵĶ��ص�����handle_input��
	//�ýӿڲ����жϷ���ֵ�Ƿ�ɹ���
	RegisterSock( m_acceptor, READ_MASK );
	return 0;
}

int CEchoSvr::handle_input( int handle )
{
	if( handle == m_acceptor.GetHandle() ){		//�������������������󵽴
		char ip_str[256];
		CSockAddrIPv4 remote_addr;
		//����������ʱ��ȡ���Ѿ�ע��������¼�
		//UnregisterSock( m_acceptor, READ_MASK | WRITE_MASK );
		CCommonStream* stream = m_acceptor.Accept( &remote_addr );
		if( stream != NULL ){			//Accept�ɹ����ж�������ָ�벻ΪNULL��		
			fprintf( stdout, "�ɹ�����һ������! ���ԣ� %s:%d\n", 
					remote_addr.GetIpStr(ip_str, sizeof(ip_str)), remote_addr.GetPort() );
			//ע���µ�socket�Ķ��¼���׼�����տͻ��˵���Ϣ��5�볬ʱ��
			add_stream_to_store( stream );
			RegisterSock( *stream, READ_MASK, 5000000 );
			//RemoveSock( m_acceptor );
			//m_acceptor.Close();		//����һ���򵥵ķ�������ֻ����һ�Σ��Ͳ������ˡ���ע���ٹص���
			return 0;
		}else{
			fprintf(stdout," accept connection failed!\n");
		}
		return 0;
	}else{//if( m_stream != NULL && handle == m_stream->GetHandle() ){  //�������ϵĿͻ��������ݵ��
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
			//delete m_stream, m_stream = NULL;		//������������
			return 0;								//�Ѿ�ɾ��socket, ����ֵ�������塣
		}else if( ret == 0 ){
			fprintf( stdout, "not recv any data, go on!\n" );
			return 0;	//����0����ʾ������5�볬ʱ�ȴ��ͻ��˵����ݡ�
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
				return 3000000;		//���أ���ʾ��������3�볬ʱ�ȴ��ͻ��˵����ݡ�
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
		delete m_stream, m_stream = NULL;       //������������
		return 0;                               //�Ѿ�ɾ��socket, ����ֵ��������
	}
	*/
	fprintf( stderr, "���󣬲��������е����!\n" );
	return 0;
}

int CEchoSvr::handle_exception( int handle )
{
	printf( "handle input timeout!\n" );
	remove_stream_from_store( handle );
	fprintf( stderr, "���󣬲��������е����!\n" );
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
	
	// �������q �ַ��˳�
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