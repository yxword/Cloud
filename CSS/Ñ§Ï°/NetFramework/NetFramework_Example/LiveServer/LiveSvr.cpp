#include <stdio.h>
#include "LiveSvr.h"
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

CLiveSvr::CLiveSvr()
:m_acceptor(NULL),
m_mediabuf(NULL)
{

}

CLiveSvr::~ CLiveSvr()
{

}

int CLiveSvr::StartSvr( CSockAddr * local )
{	
	m_acceptor = new CSockAcceptor;
	if( m_acceptor->Open( *local ) < 0 ){
		printf( "build listen socket error!\n" );
		return -1;
	}
	m_timer = SetTimer( 40*1000 );//40毫秒产生一帧数据
	m_mediabuf = new CMediaBuffer;
	RegisterSock( *m_acceptor, READ_MASK );
	return 0;
}

int CLiveSvr::StopSvr()
{
	DestroyTimer( m_timer );
	std::list<CMediaStreamSender*>::iterator it;
	for( it = m_sender_list.begin(); it != m_sender_list.end(); it++ ){
		CMediaStreamSender* tmp = *it;
		m_mediabuf->DelSender( *it );
		CSock* sock = tmp->Detach();
		tmp->Close();
		delete sock;
	}
	return 0;
}

int CLiveSvr::handle_input( int handle )
{
	if( handle == m_acceptor->GetHandle() ){ //接收客户端服务请求
		CSockAddrIPv4 remote;
		CSockStream *sock = m_acceptor->Accept( &remote );		
		CMediaStreamSender *sender = CMediaStreamSender::Create();		
		sender->Attach( sock );
		m_sender_list.push_back( sender );
		m_mediabuf->AddSender( sender );
		sender->WaitException( GetID() );
		return 0;
	}else
		return -1;
}

int64_t CLiveSvr::handle_timeout( long id )
{
	if( id == m_timer )	{
		unsigned char *buf = new unsigned char[32*1024];  //数据缓冲必须是new出来的
		CPacket *pkt = new CPacket;
		pkt->m_buf = buf;
		memset( buf, 0, 32*1024 ); //填充内容
		m_mediabuf->Put( pkt, 32*1024, 0, 2 );
		buf = new unsigned char[10*1024];
		memset( buf, 1, 10*1024 );	
		pkt = new CPacket;
		pkt->m_buf = buf;
		m_mediabuf->Put( pkt, 10*1024, 1, 2 ); //一帧结束
		return 0;
	}else
		return -1;
}

int CLiveSvr::handle_message( long from_id, MSG_TYPE type, long attach )
{
	if( type == MSG_SOCK_EXCEPTION ){  //sock错误，关闭对应的连接
		std::list<CMediaStreamSender*>::iterator it;
		for( it = m_sender_list.begin(); it != m_sender_list.end(); it++ ){
			if( from_id == (*it)->GetID() ){
				m_mediabuf->DelSender( *it );
				CSock* sock = (*it)->Detach();				
				delete sock;
				(*it)->Close();
				return 0;
			}
		}
	}
	return 0;
}

int CLiveSvr::handle_close(CNetHandler *myself)
{
	delete myself;
}

#if 0
#endif 

int StartLiveSvr()
{	
	CLiveSvr *live_svr = new CLiveSvr;
	CSockAddrIPv4 local( INADDR_ANY, 15004 );
	if( live_svr->StartSvr( &local ) < 0 )
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
	
	if (live_svr)
		live_svr->Close();
		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}

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
