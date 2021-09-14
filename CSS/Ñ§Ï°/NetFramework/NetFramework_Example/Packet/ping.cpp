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
	//1�� ����ARP����ȡ�Զ�MAC��ַ��
	m_state = 1;			//״̬Ϊ1����ARP״̬��
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
	
	RegisterSock( m_packet, READ_MASK, 3000000 );			//�ȴ�ARPӦ��3�볬ʱ��
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
				return 0;						//δ��ȡ��ȷ��ARPӦ�����¿�ʼ3�볬ʱ�ȴ���
			UnregisterSock( m_packet, READ_MASK );
			//״̬ת�������´�packet.
			m_state = 2;				//״̬1����Ϊicmp״̬�ˡ�
			if( m_packet.Open( "eth0", ETH_P_IP, IPPROTO_ICMP ) == -1 ){
				fprintf( stderr, "Open network interface error!\n" );
				RemoveSock( m_packet );
				return -1;			//m_packet�Ѿ�ע���������������ȷ���塣
			}
			m_packet.SetRemoteMac( recv_remote_mac );	//ͨ��ARP��ȡ���Զ�MAC��ַ��
			m_packet.SetRemoteIP( recv_remote );	//�Զ�IP��
			SetTimer( 1000 );	//ͨ����ʱ��������ping����.
			return -1;						//3�볬ʱ������Ҫ������ʵ�Ѿ�ע������
		}else{
			ret = m_packet.Recv( recv_buf, buf_len, &recv_local, &recv_remote, recv_local_mac, recv_remote_mac );
			if( ret != IPPROTO_ICMP )
				return 0;					//δ��ȡ��ȷ��icmpӦ�����¿�ʼ3��ȴ���
			//��ʾ�����
			char tmp_buf[256];	
			struct iphdr* iphdr = (struct iphdr *)recv_buf;
			struct icmphdr* icmphdr = (struct icmphdr *)( recv_buf + sizeof(struct iphdr) );
			fprintf( stdout, "%d bytes from %s: icmp_seq=%d ttl=%d\n", buf_len-sizeof(struct iphdr),
									recv_remote.GetIpStr( tmp_buf, sizeof(tmp_buf) ), ntohs(icmphdr->un.echo.sequence),
									iphdr->ttl );
			
			UnregisterSock( m_packet, READ_MASK );	//ÿ�յ�һ��Ӧ��ͽ������ɶ�ʱ����ʼ��һ�Ρ�
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
	RegisterSock( m_packet, READ_MASK, 3000000 );			//�ȴ�ICMPӦ��3�볬ʱ��
	return -1;																				//ע����ʱ����	
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
	
	if (pingTest)
		pingTest->Close();
		
	Infra::CThread::sleep(2 * 1000);
	
	return 0;
}