#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <netinet/ip.h>
#include "UdpPacket.h"

CUdpPacket::CUdpPacket()
{
	memset( m_broadcast_mac, 0xFF, ETH_ALEN );
	m_state = 1;
}

CUdpPacket::~CUdpPacket()
{
}

int CUdpPacket::StartSession( CSockAddrIPv4 & addr )
{
	//1、 发送ARP，获取对端MAC地址。
	m_state = 1;			//状态为1，即ARP状态。
	if( m_packet.Open( "eth0", ETH_P_ARP ) == -1 ){
		fprintf( stderr, "Open network interface error!\n" );
		return -1;
	}
	m_packet.SetRemoteIP( addr );
	m_packet.SetRemoteMac( m_broadcast_mac );
	m_packet.Send( NULL, 0 );
	
	RegisterSock( m_packet, READ_MASK, 3000000 );			//等待ARP应答，3秒超时。
	return 0;
}

int CUdpPacket::handle_input( int handle )
{
	int ret;
	char recv_buf[1500];
	memset(recv_buf, 0, sizeof(recv_buf) );
	uint32_t buf_len = sizeof(recv_buf);
	CSockAddrIPv4 recv_local, recv_remote;
	unsigned char recv_local_mac[ETH_ALEN], recv_remote_mac[ETH_ALEN];
	
	if( handle == m_packet.GetHandle() ){
		if( m_state == 1 ){
			ret = m_packet.Recv( recv_buf, buf_len, &recv_local, &recv_remote, recv_local_mac, recv_remote_mac );
			if( ret != ETH_P_ARP ){
				return 0;						//未获取正确的ARP应答，重新开始3秒超时等待。
			}
			UnregisterSock( m_packet, READ_MASK );
			//状态转换，重新打开packet.
			m_state = 2;				//状态1，即为udp状态了。
			if( m_packet.Open( "eth0", ETH_P_IP, IPPROTO_UDP ) == -1 ){
				fprintf( stderr, "Open network interface error!\n" );
				RemoveSock( m_packet );
				return -1;					//m_packet已经注销，这个返回无明确意义。
			}
					
			m_packet.SetRemoteMac( recv_remote_mac );	//通过ARP获取到对端MAC地址。
			recv_remote.SetPort( 12345 );						//设置对端udp端口。
			m_packet.SetRemoteIP( recv_remote );	//对端IP。
			recv_local.SetPort( 54259 );					//设置本地UDP端口。
			m_packet.SetLocalIP( recv_local );		//本地IP地址。		
			SetTimer( 1000 );
			return -1;						//3秒超时不再需要。（其实已经注销）。
		}else{
			ret = m_packet.Recv( recv_buf, buf_len, &recv_local, &recv_remote, recv_local_mac, recv_remote_mac );
			if( ret != IPPROTO_UDP )
				return 0;					//未获取正确的udp应答，重新开始3秒等待。
			//显示输出。
			char tmp_buf[256];	
			memset( tmp_buf, 0, sizeof(tmp_buf) );
			fprintf( stdout, "%d bytes from %s: %s\n", buf_len-sizeof(struct iphdr),
									recv_remote.GetIpStr( tmp_buf, sizeof(tmp_buf) ), 
									recv_buf+sizeof(struct iphdr)+sizeof(struct udphdr) );
			
			UnregisterSock( m_packet, READ_MASK );	//每收到一次应答就结束，由定时器开始下一次。
			SetTimer( 1000000 );
			return -1;
		}
	}
	return 0;
}

int64_t CUdpPacket::handle_timeout( long id )
{
	/*
	fprintf( stdout, "IF index: %d\n", m_packet.GetIFIndex() );
	unsigned char mac[ETH_ALEN];
	m_packet.GetLocalMac( mac, sizeof(mac) );
	for( int i = 0; i < ETH_ALEN; i ++ )
		fprintf( stdout, "%02x:", mac[i] );
	fprintf( stdout, "\n" );
	m_packet.GetRemoteMac( mac, sizeof(mac) );
	for( int i = 0; i < ETH_ALEN; i ++ )
		fprintf( stdout, "%02x:", mac[i] );
	fprintf( stdout, "\n" );
	CSockAddrIPv4 ip;
	char buf_ip[32];
	m_packet.GetLocalIP( ip );
	fprintf( stdout, "local IP: %s:%d\n", ip.GetIpStr(buf_ip, sizeof(buf_ip)), ip.GetPort() );
	m_packet.GetRemoteIP( ip );
	fprintf( stdout, "local IP: %s:%d\n", ip.GetIpStr(buf_ip, sizeof(buf_ip)), ip.GetPort() );
	*/
	char buf[256];
	memset( buf, 'a', sizeof(buf) );
	m_packet.Send( buf, sizeof(buf) );
	
	RegisterSock( m_packet, READ_MASK, 3000000 );			//等待ICMP应答，3秒超时。
	return -1;																				//注销定时器。	
}

int CUdpPacket::handle_input_timeout( int handle )
{
	if( m_state == 1 ){
		fprintf( stderr, "network timeout!\n" );
		RemoveSock( m_packet );
		return -1;
	}else{
		fprintf( stderr, "udp reply timeout!\n" );
		UnregisterSock( m_packet, READ_MASK );
		SetTimer( 1000000 );
		return -1;
	}
	return -1;
}

int CUdpPacket::handle_exception( int handle )
{
	if( handle == m_packet.GetHandle() ){
		fprintf( stderr, "packet socket error!\n" );
		RemoveSock( m_packet );
		return -1;
	}
	return -1;
}


int CUdpPacket::handle_close(CNetHandler *myself)
{
	delete myself;
}

#if 0
#endif 

int StartUdpPacket()
{
	CUdpPacket udppkt = new CUdpPacket;		
	CSockAddrIPv4 addr( "10.6.5.215", 0 ); 	
	if( udppkt->StartSession( addr ) < 0 )
	{	
		printf("");
		udppkt->Close();
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
	
	if (udppkt)
		udppkt->Close();
		
	Infra::CThread::sleep(2 * 1000);
	
	return 0;
}