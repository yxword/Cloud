#include "TcpSvr.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "Infra/Time.h"
#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"

CTcpSvr::CTcpSvr()
	:m_stream(NULL)
{
}

CTcpSvr::~CTcpSvr()
{
	if( m_stream != NULL )
		delete m_stream, m_stream = NULL;
}

int CTcpSvr::StartSvr( CSockAddrIPv4 & addr )
{	
	if( m_acceptor.Open( addr ) < 0 ){
		fprintf( stderr, "���������׽���ʧ�ܣ�\n" );
		return -1;
	}	
	RegisterSock( m_acceptor, READ_MASK );
	return 0;
}

int CTcpSvr::handle_input( int handle )
{
	if( handle == m_acceptor.GetHandle() ){		
		if( m_stream != NULL ){
			fprintf( stderr, "����һ���򵥵ķ�������ֻ����һ�����ӣ�\n" );
			return -1;
		}
		char ip_str[256];
		CSockAddrIPv4 remote_addr;
		m_stream = m_acceptor.Accept( &remote_addr );
		if( m_stream != NULL ){			//Accept�ɹ����ж�������ָ�벻ΪNULL��		
			fprintf( stdout, "�ɹ�����һ������! ���ԣ� %s:%d\n", 
					remote_addr.GetIpStr(ip_str, sizeof(ip_str)), remote_addr.GetPort() );
			
			RegisterSock( *m_stream, READ_MASK, 5000000 );
			RemoveSock( m_acceptor );
			m_acceptor.Close();		
		}
		return 0;
	}else if( m_stream != NULL && handle == m_stream->GetHandle() ){  
		char recv_buf[1024];
		memset( recv_buf, 0, sizeof(recv_buf) );
		int ret = m_stream->Recv( recv_buf, sizeof(recv_buf) );
		if( ret < 0 ){
			if( errno == ECONNRESET )
				fprintf( stderr, "���ӱ��Է��Ͽ�\n" );
			else
				fprintf( stderr, "���մ���\n" );
			RemoveSock( *m_stream );
			delete m_stream, m_stream = NULL;	
			return 0;								
		}else if( ret == 0 ){
			fprintf( stdout, "û�н��յ��κ����ݣ�������\n" );
			return 0;	
		}else{
			fprintf( stdout, ": %s\n", recv_buf );
			return 3000000;		
		}
	}
	fprintf( stderr, "���󣬲��������е������\n" );
	return 0;			
}

int CTcpSvr::handle_input_timeout( int handle )
{
	if( m_stream != NULL && handle == m_stream->GetHandle() ){
		fprintf( stderr, "�ȴ��ͻ��˵����ݳ�ʱ!\n" );
		RemoveSock( *m_stream );
		delete m_stream, m_stream = NULL;      
		return 0;                              
	}
	fprintf( stderr, "���󣬲��������е����!\n" );
	return 0;
}

int CTcpSvr::handle_close(CNetHandler *myself)
{
	delete myself;
	return 0;
}

#if 0
#endif 

int StartTcpSvr()
{
	CTcpSvr *ptcpsvr = new CTcpSvr;		
	CSockAddrIPv4 addr( INADDR_ANY, 12345 ); 	
	if( ptcpsvr->StartSvr( addr) < 0 )
	{	
		printf("start svr failed\n");
		return -1;
	}
	
	// �������q �ַ��˳�
	char c = 0;
	while((c = getchar()))
	{
		if ('q' == c)
		{
			printf("exit \n");
			break;
		}

		Dahua::Infra::CThread::sleep(1 * 1000);
	}
	
	if (ptcpsvr)
		ptcpsvr->Close();
		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}
