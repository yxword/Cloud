#include "Multicast.h"

#include <stdio.h>
#include <string.h>

CMulticast::CMulticast()
{
}

CMulticast::~CMulticast()
{
}

int CMulticast::StartSvr( CSockAddr & addr )
{
	//传入的必须是一个组播地址，需要进行判断。
	m_mc_addr = *((CSockAddrIPv4 *)&addr);

	if( m_dgram.Open( &addr ) < 0 ){
		fprintf( stderr, "绑定UDP地址失败！\n" );
		return -1;
	}

	if( m_dgram.SetMulticastTTL( 3 ) < 0 ){
		printf( "设置组播TTL失败!\n" );
		return -1;
	}
	//设置回送，以让自己也能接收到。
	if( m_dgram.SetMulticastLoop( true ) < 0 ){
		printf( "设置组播回送失败！\n" );
		return -1;
	}

	RegisterSock( m_dgram, READ_MASK, 5000000 );
	m_timer_id = SetTimer( 3000000 );					//3秒发送一个数据报。
	return 0;
}

int64_t CMulticast::handle_timeout( long id )
{
	char send_buf[256];
	strncpy( send_buf, "这是一个组播数据报!", sizeof(send_buf) );
	if( id == m_timer_id ){
		if( m_dgram.Send( send_buf, strlen(send_buf), &m_mc_addr ) < 0 ){
			fprintf( stderr, "udp 发送错误!\n" );
			RemoveSock( m_dgram );
			return -1;
		}
		return 0;			//维持原来的定时器。
	}
	return 0;
}

int CMulticast::handle_input( int handle )
{
	char recv_buf[1024];
	CSockAddrIPv4 addr;
	char ipStr[256];
	int ret = 0;
	if( handle == m_dgram.GetHandle() ){
		ret = m_dgram.Recv( recv_buf, sizeof(recv_buf), &addr );
		if( ret < 0 ){
			fprintf( stderr, "接收udp数据报失败!\n" );
			RemoveSock( m_dgram );
			return -1;
		}else if( ret > 0 ){
			fprintf( stdout, "接收到来自%s:%d的数据: %s\n", 
					addr.GetIpStr(ipStr, sizeof(ipStr)), addr.GetPort(), recv_buf );
			return 0;		//维持原来超时。
		}
		return 0;
	}
	return 0;
}

int CMulticast::handle_exception( int handle )
{
	if( handle == m_dgram.GetHandle() ){
		fprintf( stderr, "组播udp socket异常!\n" );
		RemoveSock( m_dgram );
		return -1;
	}
	return -1;
}

int CMulticast::handle_input_timeout( int handle )
{
	if( handle == m_dgram.GetHandle() ){
		fprintf( stderr, "组番udp socket超时!\n" );
		RemoveSock( m_dgram );
		return -1;
	}
	return -1;
}

int CMulticast::handle_close(CNetHandler *myself)
{
	delete myself;
}

#if 0
#endif 

int StartMulticast()
{
	CMulticast mult = new CMulticast;
	CSockAddrIPv4 addr( "224.0.1.2", 5000 );
	if( mult->StartSvr( addr ) < 0 )
		return -1;
		
	// 如果输入q 字符退出
	char c = 0;
	while(c = getchar())
	{
		if ('q' == c)
		{
			DLOG_INFO("exit \n");
			break;
		}

		Infra::CThread::sleep(1 * 1000);
	}
	
	if (mult)
		mult->Close();
		
	Infra::CThread::sleep(2 * 1000);
	
	return 0;
}
