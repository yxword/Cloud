#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "ping.h"

CPing::CPing()
{
	m_icmphdr.type = ICMP_ECHO;
	m_icmphdr.code = 0;
	m_icmphdr.un.echo.id = getpid();
	m_icmphdr.un.echo.sequence = 0;
	
	memset( m_broadcast_mac, 0xFF, ETH_ALEN );
	m_state = 0;
	m_seq = 0;
}

CPing::~CPing()
{
}

int CPing::StartPing( CSockAddrIPv4& addr )
{	
	//1、 发送ARP，获取对端MAC地址。
	m_state = 1;			//状态为1，即ARP状态。
	if( m_packet.Open( "eth0", ETH_P_ARP ) == -1 ){
		fprintf( stderr, "Open network interface error!\n" );
		return -1;
	}
	char tmp_buf[256];
	addr.GetIpStr( tmp_buf, sizeof(tmp_buf) );
	printf( "CPing::StartPing, addr:%s!\n", tmp_buf );	
	m_packet.SetRemoteIP( addr );
	m_packet.SetRemoteMac( m_broadcast_mac );
	m_packet.Send( NULL, 0 );
	
	RegisterSock( m_packet, READ_MASK, 3000000 );			//等待ARP应答，3秒超时。
	return 0;
}

int CPing::handle_input( int handle )
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
			if( ret != ETH_P_ARP )
				return 0;						//未获取正确的ARP应答，重新开始3秒超时等待。
			UnregisterSock( m_packet, READ_MASK );
			//状态转换，重新打开packet.
			m_state = 2;				//状态1，即为icmp状态了。
			if( m_packet.Open( "eth0", ETH_P_IP, IPPROTO_ICMP ) == -1 ){
				fprintf( stderr, "Open network interface error!\n" );
				RemoveSock( m_packet );
				return -1;			//m_packet已经注销，这个返回无明确意义。
			}
			m_packet.SetRemoteMac( recv_remote_mac );	//通过ARP获取到对端MAC地址。
			m_packet.SetRemoteIP( recv_remote );	//对端IP。
			SetTimer( 1000 );	//通过定时器来启动ping流程.
			return -1;						//3秒超时不再需要。（其实已经注销）。
		}else{
			ret = m_packet.Recv( recv_buf, buf_len, &recv_local, &recv_remote, recv_local_mac, recv_remote_mac );
			if( ret != IPPROTO_ICMP )
				return 0;					//未获取正确的icmp应答，重新开始3秒等待。
			//显示输出。
			char tmp_buf[256];	
			struct iphdr* iphdr = (struct iphdr *)recv_buf;
			struct icmphdr* icmphdr = (struct icmphdr *)( recv_buf + sizeof(struct iphdr) );
			fprintf( stdout, "%d bytes from %s: icmp_seq=%d ttl=%d\n", buf_len-sizeof(struct iphdr),
									recv_remote.GetIpStr( tmp_buf, sizeof(tmp_buf) ), ntohs(icmphdr->un.echo.sequence),
									iphdr->ttl );
			
			UnregisterSock( m_packet, READ_MASK );	//每收到一次应答就结束，由定时器开始下一次。
			SetTimer( 1000000 );
			return -1;
		}
	}
	return 0;
}

int64_t CPing::handle_timeout( long id )
{
	char buf[256];
	memset( buf, 0, sizeof(buf) );
	m_icmphdr.un.echo.sequence = htons(m_seq);
	m_seq++;
	memcpy( buf, &m_icmphdr, sizeof(m_icmphdr) );
	
	m_packet.Send( buf, sizeof(buf) );
	RegisterSock( m_packet, READ_MASK, 3000000 );			//等待ICMP应答，3秒超时。
	return -1;																				//注销定时器。	
}

int CPing::handle_input_timeout( int handle )
{
	if( m_state == 1 ){
		fprintf( stderr, "network timeout!\n" );
		RemoveSock( m_packet );
		return -1;
	}else{
		fprintf( stderr, "ping reply timeout!\n" );
		UnregisterSock( m_packet, READ_MASK );
		SetTimer( 1000000 );
		return -1;
	}
	return -1;
}

int CPing::handle_exception( int handle )
{
	if( handle == m_packet.GetHandle() ){
		fprintf( stderr, "packet socket error!\n" );
		RemoveSock( m_packet );
		return -1;
	}
	return -1;
}

int CPing::handle_close(CNetHandler *myself)
{
	delete myself;
}

#if 0
#endif 

int StartCpingTest()
{
	CPing pingTest = new CPing;		
	CSockAddrIPv4 addr( "10.6.5.215", 0 ); 	
	if( pingTest->StartPing( addr ) < 0 )
	{	
		printf("");
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
	
	if (pingTest)
		pingTest->Close();
		
	Infra::CThread::sleep(2 * 1000);
	
	return 0;
}