#include "UdpSvr.h"
#include "NetFramework/SockAddrIPv4.h"

#include <stdio.h>
#include <string.h>

CUdpSvr::CUdpSvr()
{
}

CUdpSvr::~CUdpSvr()
{
}

int CUdpSvr::StartSvr( CSockAddr & addr )
{
	if( m_dgram.Open( &addr ) < 0 ){
		fprintf( stderr, "绑定UDP地址失败！\n" );
		return -1;
	}
	RegisterSock( m_dgram, READ_MASK, 5000000 );
	return 0;
}

int CUdpSvr::handle_input( int handle )
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

int CUdpSvr::handle_exception( int handle )
{
	if( handle == m_dgram.GetHandle() ){
		fprintf( stderr, "udp socket异常!\n" );
		RemoveSock( m_dgram );
		return -1;
	}
	return -1;
}

int CUdpSvr::handle_input_timeout( int handle )
{
	if( handle == m_dgram.GetHandle() ){
		fprintf( stderr, "udp socket超时!\n" );
		RemoveSock( m_dgram );
		return -1;
	}
	return -1;
}

int CUdpSvr::handle_close(CNetHandler *myself)
{
	delete myself;
}

#if 0
#endif 

int StartUdpSvr()
{
	CUdpSvr udpsvr = new CUdpSvr;		
	CSockAddrIPv4 addr( INADDR_ANY, 12345 ); 	
	if( udpsvr->StartSvr( addr ) < 0 )
	{	
		printf("StartSvr failed\n");
		return -1;
	}
	
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
	
	if (udpsvr)
		udpsvr->Close();
		
	Infra::CThread::sleep(2 * 1000);
	
	return 0;
}