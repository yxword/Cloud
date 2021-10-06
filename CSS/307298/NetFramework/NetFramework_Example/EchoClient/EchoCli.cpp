#ifdef _USE_OPENSSL
#include "SslX509.h"
#endif
#include "EchoCli.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "Infra/Thread.h"
 
CEchoCli::CEchoCli()
{
}

CEchoCli::~CEchoCli()
{
}

int CEchoCli::StartSession( CSockAddr & addr )
{
	//����ж�û�б�Ҫ��Connect��������жϣ�д��������Ϊ�˰������õ�����ַ����
	if( addr.GetType() != CSockAddr::SOCKADDR_TYPE_IPV4 ){
		fprintf( stderr, "��ַ���ʹ���\n");
		return -1;
	}
#ifdef _USE_OPENSSL
	CSslX509* x509 = new CSslX509;
	if( x509->SetTrustCA("./cert/RootCA.crt") ){
		delete x509, x509=NULL;
		printf("set certificate failed!\n");
		return -1;
	}else
		printf("set certificate succeed!\n");
	//m_stream.SetOption( Module_X509, Type_X509_verify, x509 );
#endif
	//Connect����-1��ֱ��ʧ�ܣ������ش��ڵ���0��ʾ�ɹ���
	if( m_stream.Connect( addr ) < 0 ){
		printf( "����ֱ��ʧ�ܣ�\n" );
		return -1;
	}
	printf("Connection succeed!\n");
	//ע��д�¼�
	do_send();
	RegisterSock( m_stream, WRITE_MASK, 5000000 );
	return 0;
}
int CEchoCli::handle_input( int handle )
{
	//printf( "run in handle input!\n" );
	if( handle == m_stream.GetHandle() ){
#define RECV_BUFFER_SIZE 1024*16+1
		char recv_buf[RECV_BUFFER_SIZE];
		memset( recv_buf, 0, sizeof(recv_buf) );
		int ret = m_stream.Recv( recv_buf, sizeof(recv_buf) );
		if( ret < 0 ){
			if( errno == ECONNRESET )
				fprintf( stderr, "link be closed!\n" );
			else
				fprintf( stderr, "recv error!\n" );
			RemoveSock( m_stream );
			return 0;						//�Ѿ�ɾ��socket, ����ֵ�������塣
		}else if( ret == 0 ){
			fprintf( stdout, "not recv any data, go on!\n" );
			return 0;	//����0����ʾ������5�볬ʱ�ȴ��ͻ��˵����ݡ�
		}else{
			//fprintf( stdout, "echo server recv data len: %d\n", ret );
			UnregisterSock( m_stream, READ_MASK );
			RegisterSock( m_stream, WRITE_MASK );
			return 3000000;		//���أ���ʾ��������3�볬ʱ�ȴ��ͻ��˵����ݡ�
		}
	}
	return 0;
}
int CEchoCli::handle_output( int handle )
{
	//���m_stream�ڷ������ݵĹ�������Ϊ������ȥע����д�¼���
	//printf( "run in handle_output!\n" );
	if( handle == m_stream.GetHandle() ){		
	//	printf( "goto send data!\n" );
		do_send();
		UnregisterSock( m_stream, WRITE_MASK );
		RegisterSock( m_stream, READ_MASK );
	}
	return 0;
}

int CEchoCli::do_send()
{
	static unsigned long send_count = 0;
	char send_buf[RECV_BUFFER_SIZE];
	memset( send_buf, 'h', sizeof send_buf );
	send_buf[RECV_BUFFER_SIZE - 1 ] = '\0';
	//strncpy( send_buf, "hello world!\n", sizeof(send_buf) );
	int ret = m_stream.Send( send_buf, strlen(send_buf) );
	if( ret < 0 )
		fprintf( stderr, "send data error!\n" );
	else if ( ret < strlen(send_buf) - 1 )
		printf( "send %d bytes!\n", ret );
	else if( send_count++ % 10000 == 0 ){
		printf("EchoCli do_send succeed! %ld\n", send_count-1 );
	}
	return ret;
}

int CEchoCli::handle_output_timeout( int handle )
{
	printf( "timeout!\n" );
	if( handle == m_stream.GetHandle() ){
		printf( "link is timeout!\n" );
		RemoveSock( m_stream );
	}
	return -1;			//��ʱ����һ�ɲ��ٱ�����
}

int CEchoCli::handle_exception( int handle )
{
	if( handle == m_stream.GetHandle() ){
		printf( "���ӷ����쳣!\n" );
		RemoveSock( m_stream );
	}
	return -1;
}


int CEchoCli::handle_close(CNetHandler *myself)
{
	delete myself;
}

#if 0
#endif 

int StartEchoCli(int argc, char **argv)
{
	CSockAddrIPv4 addr( "127.0.0.1", 12345 );
	int client_count = 1;
	if( argc >= 2 )
		client_count  = atoi(*(argv+1));
	printf("client_count :%d \n", client_count);
	
	CEchoCli** cli = new CEchoCli*[client_count];
	for( int i = 0; i < client_count; i++ ){
		cli[i] = new CEchoCli;
		if( cli[i]->StartSession( addr ) < 0 )
			printf(" %d clinet connection failed!\n", i);
		else 
			printf(" %d clinet connection succeed!\n", i);
	}
	
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
	
	CEchoCli* pcli;
	for (int i = 0; i < client_count; i++)
	{
		pcli = cli[i];
		pcli->Close();
	}

	delete []cli, cli=NULL;
		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}