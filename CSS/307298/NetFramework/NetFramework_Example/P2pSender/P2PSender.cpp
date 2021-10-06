#include <stdio.h>
#include "P2PSender.h"
#include "NetFramework/SockAddrIPv4.h"
#include "Infra/Thread.h"
CP2PSender::CP2PSender()
:m_sender(NULL),
m_stream(NULL)
{

}

CP2PSender::~CP2PSender()
{

}

int CP2PSender::StartSender( CSockAddr &local )
{
	if( m_acceptor.Open( local ) < 0 ){ //侦听端口，等待建立TCP连接
		printf( "open listen socket error!\n" );
		return -1;
	}
	RegisterSock( m_acceptor, READ_MASK );	
	return 0;
}

int CP2PSender::StopSender()
{
	UnregisterSock( m_acceptor, READ_MASK ); 
	DestroyTimer( m_timer ); 	
	if( m_sender != NULL ){
		m_sender->Clear(); 		
		m_sender->Detach();		
		m_sender->Close();		
	}
	if( m_stream != NULL )
		delete m_stream, m_stream = NULL;	
	return 0;
}
	
int CP2PSender::handle_input( int handle )
{
	if( handle == m_acceptor.GetHandle() ){
		CSockAddrIPv4 remote;
		if( m_stream == NULL ){
			printf( "recv a connect!\n" );
			m_stream = m_acceptor.Accept( &remote ); //建立连接，只允许一个连接
		}else{
			printf("connect refuse!\n");
			//UnregisterSock( m_acceptor, READ_MASK );
			return -1;					
		}
		if( m_stream != NULL ){			
			m_sender = CStreamSender::Create();
			m_sender->SetBufferSize( 10 );
			m_sender->Attach( this, m_stream );
			m_sender->WaitException( GetID() );
			m_timer = SetTimer( 40*1000 );  //建立定时器，在handle_timeout中模拟媒体的产生过程
		}
	}else
		return -1;
	return 0;
}

int64_t CP2PSender::handle_timeout( long id )
{
	if( id == m_timer ){
		char buf[36*1024];
		memset( buf, 1, sizeof(buf) );
		int ret = m_sender->Put( buf, sizeof(buf) );
		if( ret < 0 ){
			m_sender->WaitBufferSize( GetID(), sizeof(buf) ); //缓冲已满，等待足够的缓冲空间，注销定时器
			return -1;
		}else
			return 0;
	}else
		return -1;
}

int CP2PSender::handle_message( long from_id, MSG_TYPE type, long attach )
{
	if( type == MSG_BUFFER_SIZE ){
		m_timer = SetTimer( 40*1000 ); //缓冲已空闲，重新设置定时器，中间会有一段时间间隔
		return 0;
	}else if( type == MSG_SOCK_EXCEPTION ){
		printf( "sock error!\n" );
		StopSender();
		return 0;
	}else{
		return -1;
	}	
}

int CP2PSender::handle_output( int handle )
{
	if( m_sender != NULL )
		return m_sender->handle_output( handle );
	else
		return 0;
}


int CP2PSender::handle_close(CNetHandler *myself)
{
	delete myself;
}

#if 0
#endif 

int StartP2pSenderTest()
{
	CP2PSender *sender = new CP2PSender;
	
	CSockAddrIPv4 local( INADDR_ANY, 15004 );
	if( sender->StartSender( local ) < 0 )
		return -1;
	sleep( 30 );
	sender->StopSender();
	
	// 如果输入q 字符退出
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
	
	if (sender)
		sender->Close();
		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}