#include <stdio.h>
#include "LiveCli.h"
#include "NetFramework/SockAddrIPv4.h"
#include "Infra/Thread.h"

class CPacket : public CMediaPacket
{
public:
	CPacket(){}
	virtual ~CPacket(){}
public:
	virtual void Release(){ delete []m_buf; delete this; }
	virtual unsigned char *GetBuffer(){ return m_buf;}
public:
	unsigned char*	m_buf;
};

/*******************************************************/
CLiveCli::CLiveCli()
{

}

CLiveCli::~CLiveCli()
{

}

int CLiveCli::StartWatch( CSockAddr *remote )
{
	if( m_stream.Connect( *remote ) < 0 ){
		printf( "connect error!\n" );
		return -1;
	}
	RegisterSock( m_stream, WRITE_MASK, 5*1000*1000 );
	return 0;
}

int CLiveCli::handle_output( int handle )
{
	if( handle == m_stream.GetHandle() ){
		int ret = m_stream.GetConnectStatus();
		if( ret == CSockStream::STATUS_NOTCONNECTED ){
			printf( "connecting!\n" );
			return -1;
		}else if( ret == CSockStream::STATUS_ERROR ){
			printf( "connect error!\n" );
			return -1;
		}else if( ret == CSockStream::STATUS_CONNECTED ){
			printf( "connect success!\n" );
			UnregisterSock( m_stream, WRITE_MASK );
			RegisterSock( m_stream, READ_MASK ); 
			return 0;
		}else
			return -1;
	}
	else
		return -1;
}

int CLiveCli::handle_input( int handle )
{
	if( handle == m_stream.GetHandle() ){
		char buf[32*1024];
		int ret = m_stream.Recv( buf, sizeof(buf) );
		if( ret > 0 )
			printf( "the signal is perfect!\n" );
		else{
			printf( "Recv error!\n" );
			UnregisterSock( m_stream, READ_MASK );
		}
	}
	return 0;
}


int CLiveCli::handle_close(CNetHandler *myself)
{
	delete myself;
}

#if 0
#endif 

int StartLiveCli()
{	
	int numClient = 1;
	
	CLiveCli **cli = new CLiveCli *[numClient];
	CSockAddrIPv4 remote( "127.0.0.1", 15004 );
	for( int i = 0; i < numClient; i++ ){
		cli[i] = new CLiveCli;
		if( cli[i]->StartWatch( &remote ) < 0 )
			return -1;
	}
	
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
	
	for (int i = 0; i < numClient; i++)
	{
		cli[i]->Close();
	}
	
	delete []cli, cli=NULL;
		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}
