#include "SockProtSvr.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "Infra/Thread.h"

CSockProtOther::CSockProtOther()
{
}

CSockProtOther::~CSockProtOther()
{
}

int CSockProtOther::RegisterSk( CSock& socket, SOCK_MASK type )
{
	return RegisterSock(socket, type);
}

int CSockProtOther::UnregisterSk(CSock& socket, SOCK_MASK type)
{
	return UnregisterSock(socket, type);
}

int CSockProtOther::RemoveSk(CSock& socket)
{
	return RemoveSock(socket);
}

CSockProtSvr::CSockProtSvr()
{
	m_stream = NULL;
	m_thread = NULL;
}

CSockProtSvr::~CSockProtSvr()
{
	if( m_thread != NULL ){
		if( !m_thread->isThreadOver()){
			m_thread->cancelThread();
			m_thread->destroyThread();
		}
		delete m_thread, m_thread = NULL;
	}

	if (m_spother)
		delete m_spother, m_spother = NULL;

	if( m_stream != NULL )
		delete m_stream, m_stream = NULL;
}

int CSockProtSvr::StartSvr( CSockAddrIPv4 & addr )
{	
	if( m_acceptor.Open( addr ) < 0 ){
		fprintf( stderr, "���������׽���ʧ�ܣ�\n" );
		return -1;
	}	
	RegisterSock( m_acceptor, READ_MASK|PROTECT_MASK);

	m_spother = new CSockProtOther;

	if( m_thread == NULL )
		m_thread = new Dahua::Infra::CThreadLite( Dahua::Infra::CThreadLite::ThreadProc(&CSockProtSvr::ThreadProc,this), "sock_protect_svr");

	m_thread->createThread();
		
	return 0;
}

int CSockProtSvr::handle_input( int handle )
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
			//m_acceptor.Close();		
		}
		return 0;
	}else if( m_stream != NULL && handle == m_stream->GetHandle() ){  
		char recv_buf[1024];
		memset( recv_buf, 0, sizeof(recv_buf) );
		int ret = m_stream->Recv( recv_buf, sizeof(recv_buf) );
		if( ret < 0 ){
			if( errno == -1 )
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
			return 5000000;		
		}
	}
	fprintf( stderr, "���󣬲��������е������\n" );
	return 0;			
}

int CSockProtSvr::handle_input_timeout( int handle )
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

void CSockProtSvr::ThreadProc( Dahua::Infra::CThreadLite& arg )
{
	while( arg.looping() )
	{
	#if 0
		if (!m_stream)
		{
			Dahua::Infra::CThread::sleep(2000);
			continue;
		}
	#endif
		
		Dahua::Infra::CThread::sleep(1000);
		m_spother->RegisterSk(m_acceptor, READ_MASK);

		Dahua::Infra::CThread::sleep(1000);
		m_spother->UnregisterSk(m_acceptor, READ_MASK);

		Dahua::Infra::CThread::sleep(1000);
		m_spother->RemoveSk(m_acceptor);
	}

}	


int CSockProtSvr::handle_close(CNetHandler *myself)
{
	delete myself;
}

#if 0
#endif 

int StartSockProtSvr()
{
	CSockProtSvr *spsvr = new CSockProtSvr;		
	CSockAddrIPv4 addr( INADDR_ANY, 12345 );	
	if( spsvr->StartSvr( addr ) < 0 )
		return -1;
		
	// �������q �ַ��˳�
	char c = 0;
	while(c = getchar())
	{
		if ('q' == c)
		{
			printf("exit \n");
			break;
		}

		Dahua::Infra::CThread::sleep(1 * 1000);
	}
	
	if (spsvr)
		spsvr->Close();
		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}

int CSockProtOther::handle_input(int handle)
{
	printf("CSockProtOther::handle_input occur, handle: %d\n", handle);
}



