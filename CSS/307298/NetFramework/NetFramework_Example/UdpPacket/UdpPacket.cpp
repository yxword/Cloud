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
	//1�� ����ARP����ȡ�Զ�MAC��ַ��
	m_state = 1;			//״̬Ϊ1����ARP״̬��
	if( m_packet.Open( "eth0", ETH_P_ARP ) == -1 ){
		fprintf( stderr, "Open network interface error!\n" );
		return -1;
	}
	m_packet.SetRemoteIP( addr );
	m_packet.SetRemoteMac( m_broadcast_mac );
	m_packet.Send( NULL, 0 );
	
	RegisterSock( m_packet, READ_MASK, 3000000 );			//�ȴ�ARPӦ��3�볬ʱ��
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
				return 0;						//δ��ȡ��ȷ��ARPӦ�����¿�ʼ3�볬ʱ�ȴ���
			}
			UnregisterSock( m_packet, READ_MASK );
			//״̬ת�������´�packet.
			m_state = 2;				//״̬1����Ϊudp״̬�ˡ�
			if( m_packet.Open( "eth0", ETH_P_IP, IPPROTO_UDP ) == -1 ){
				fprintf( stderr, "Open network interface error!\n" );
				RemoveSock( m_packet );
				return -1;					//m_packet�Ѿ�ע���������������ȷ���塣
			}
					
			m_packet.SetRemoteMac( recv_remote_mac );	//ͨ��ARP��ȡ���Զ�MAC��ַ��
			recv_remote.SetPort( 12345 );						//���öԶ�udp�˿ڡ�
			m_packet.SetRemoteIP( recv_remote );	//�Զ�IP��
			recv_local.SetPort( 54259 );					//���ñ���UDP�˿ڡ�
			m_packet.SetLocalIP( recv_local );		//����IP��ַ��		
			SetTimer( 1000 );
			return -1;						//3�볬ʱ������Ҫ������ʵ�Ѿ�ע������
		}else{
			ret = m_packet.Recv( recv_buf, buf_len, &recv_local, &recv_remote, recv_local_mac, recv_remote_mac );
			if( ret != IPPROTO_UDP )
				return 0;					//δ��ȡ��ȷ��udpӦ�����¿�ʼ3��ȴ���
			//��ʾ�����
			char tmp_buf[256];	
			memset( tmp_buf, 0, sizeof(tmp_buf) );
			fprintf( stdout, "%d bytes from %s: %s\n", buf_len-sizeof(struct iphdr),
									recv_remote.GetIpStr( tmp_buf, sizeof(tmp_buf) ), 
									recv_buf+sizeof(struct iphdr)+sizeof(struct udphdr) );
			
			UnregisterSock( m_packet, READ_MASK );	//ÿ�յ�һ��Ӧ��ͽ������ɶ�ʱ����ʼ��һ�Ρ�
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
	
	RegisterSock( m_packet, READ_MASK, 3000000 );			//�ȴ�ICMPӦ��3�볬ʱ��
	return -1;																				//ע����ʱ����	
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
	
	// �������q �ַ��˳�
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