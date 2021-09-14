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
  //������Կ�ļ���·��
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

void CSslSvr::StopSvr()
{
	RemoveSock(m_acceptor);
}

int CSslSvr::handle_input( int handle )
{
	if( handle == m_acceptor.GetHandle() ){		//�������������������󵽴

		if( m_stream != NULL ){
			fprintf( stderr, "just surport only one client!\n" );
			return -1;
		}
		char ip_str[256];
		CSockAddrIPv4 remote_addr;
		//����������ʱ��ȡ���Ѿ�ע��������¼�
		UnregisterSock( m_acceptor, READ_MASK | WRITE_MASK );
		m_stream = m_acceptor.Accept( &remote_addr );
		if( m_stream != NULL ){			//Accept�ɹ����ж�������ָ�벻ΪNULL��		
			fprintf( stdout, "recv a new connection! form: %s:%d\n", 
					remote_addr.GetIpStr(ip_str, sizeof(ip_str)), remote_addr.GetPort() );
			#if 0
			//ע���µ�socket�Ķ��¼���׼�����տͻ��˵���Ϣ��5�볬ʱ��
			delete m_stream, m_stream = NULL;
			RegisterSock( m_acceptor, READ_MASK | WRITE_MASK );
			return 0;
			#endif 
			RegisterSock( *m_stream, READ_MASK, 50000000 );
			
			RegisterSock( m_acceptor, READ_MASK | WRITE_MASK );
			/* 
			RemoveSock( m_acceptor );
			m_acceptor.Close();		//����һ���򵥵ķ�������ֻ����һ�Σ��Ͳ������ˡ���ע���ٹص���
			*/
			return 0;
		}
		return 0;
	}else if( m_stream != NULL && handle == m_stream->GetHandle() ){  //�������ϵĿͻ��������ݵ��
		char recv_buf[1024];
		memset( recv_buf, 0, sizeof(recv_buf) );
		int ret = m_stream->Recv( recv_buf, sizeof(recv_buf) );
		if( ret < 0 ){
			if( errno == ECONNRESET )
				fprintf( stderr, "link be closed!\n" );
			else
				fprintf( stderr, "recv error!\n" );
			RemoveSock( *m_stream );
			delete m_stream, m_stream = NULL;		//������������
			return 0;								//�Ѿ�ɾ��socket, ����ֵ�������塣
		}else if( ret == 0 ){
			fprintf( stdout, "not recv any data, go on!\n" );
			return 0;	//����0����ʾ������5�볬ʱ�ȴ��ͻ��˵����ݡ�
		}else{
			fprintf( stdout, "recv data:\n%s\n", recv_buf );
			RemoveSock( *m_stream );				// ������һ��
			delete m_stream, m_stream = NULL;
			return 0;						
			return 3000000;		//���أ���ʾ��������3�볬ʱ�ȴ��ͻ��˵����ݡ�
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
		delete m_stream, m_stream = NULL;       //������������
		return 0;                               //�Ѿ�ɾ��socket, ����ֵ��������
	}
	fprintf( stderr, "���󣬲��������е����!\n" );
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
	
	// �������"quit" �ַ��˳�
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
