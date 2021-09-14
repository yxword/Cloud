//注意：本程序是一个反面教材，是错误运用框架的一个示例，千万不要
//用于实际应用中。只有在数据发布出去的时候才应该注册写事件。
#include "WriteEvent.h"
#include "Infra/Thread.h"
#include <stdio.h>
#include <string.h>

CWriteEvent::CWriteEvent()
	:m_status(0)
{
}

CWriteEvent::~CWriteEvent()
{
}

int CWriteEvent::StartSession( CSockAddr & addr )
{	
	if( m_stream.Connect( addr ) < 0 ){
		printf( "connect failed directly！\n" );
		return -1;
	}
	RegisterSock( m_stream, WRITE_MASK, 5000000 );
	return 0;
}

int CWriteEvent::handle_output( int handle )
{
	
	if( handle == m_stream.GetHandle() ){
		if( m_status > 0 ){
			do_send();
			return 0;
		}		
		int ret = m_stream.GetConnectStatus();
		if( ret == CSockStream::STATUS_NOTCONNECTED ){
			printf( "connecting...!\n" );
			return 0;						
		}else if( ret == CSockStream::STATUS_ERROR ){
			printf( "connect failed！ \n" );
			RemoveSock( m_stream );			
			return -1;						
		}else{
			printf( "connect success!\n" );
			SetTimer( 1000000 );
			m_status = 1;
			return 0;			
		}
	}
	return 0;
}

int CWriteEvent::do_send()
{
	if( m_status == 1 ) 
		return 0;
		
	char send_buf[256];
	strncpy( send_buf, "hello world!\n", sizeof(send_buf) );
	int ret = m_stream.Send( send_buf, strlen(send_buf) );
	if( ret < 0 )
		fprintf( stderr, "send data error!\n" );
	else
		printf("send data success!\n");
	m_status = 1;
	return ret;
}

int64_t CWriteEvent::handle_timeout( long handle )
{
	m_status = 2;
	return 0;
}

int CWriteEvent::handle_output_timeout( int handle )
{
	if( handle == m_stream.GetHandle() ){
		printf( "connect timeout!\n" );
		RemoveSock( m_stream );
	}
	return -1;			
}

int CWriteEvent::handle_exception( int handle )
{
	if( handle == m_stream.GetHandle() ){
		printf( "连接发生异常!\n" );
		RemoveSock( m_stream );
	}
	return -1;
}
int CWriteEvent::handle_close(CNetHandler *myself)
{
	delete myself;
}

#if 0
#endif 

int StartWriteEvent()
{	
	CWriteEvent *write_event = new CWriteEvent;
	CSockAddrIPv4 addr( "127.0.0.1", 12345 );
	if( write_event->StartSession( addr ) < 0 )
		return -1;
	
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
	
	if (write_event)
		write_event->Close();
		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}