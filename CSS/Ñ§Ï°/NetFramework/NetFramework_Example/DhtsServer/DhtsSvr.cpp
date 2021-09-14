#include "DhtsSvr.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"

CDhtsSvr::CDhtsSvr()
    :m_conn(NULL)
{

}

CDhtsSvr::~CDhtsSvr()
{
    if (m_conn != NULL)
    {
        delete m_conn; 
        m_conn = NULL;
    }
}

int CDhtsSvr::StartSvr( CSockAddrIPv4 & addr)
{
    if ( m_acceptor.Open( addr) < 0)
    {
        printf(" start serv failed\n");
        return -1;
    }

    RegisterSock(m_acceptor, READ_MASK);

    return 0;
}


int CDhtsSvr::handle_input( int handle )
{
	if( handle == m_acceptor.GetHandle() ){		
		if( m_conn != NULL ){
			fprintf( stderr, "这是一个简单的服务器，只接收一个连接！\n" );
			return -1;
		}
		char ip_str[256];
		CSockAddrIPv4 remote_addr;
		m_conn = m_acceptor.Accept( &remote_addr );
		if( m_conn != NULL ){			//Accept成功的判断依据是指针不为NULL。		
			printf( "成功接受一个连接! 来自： %s:%d, handle:%ld\n", 
					remote_addr.GetIpStr(ip_str, sizeof(ip_str)), remote_addr.GetPort(), m_conn->GetHandle());
			
			RegisterSock( *m_conn, READ_MASK);

			//RemoveSock( m_acceptor );
			//m_acceptor.Close();		
		}
		return 0;
	}else if( m_conn != NULL && handle == m_conn->GetHandle() ){  
		char recv_buf[1024];
        int ret = 0;
		memset( recv_buf, 0, sizeof(recv_buf) );
		ret = m_conn->Recvmsg( recv_buf, sizeof(recv_buf) );
		if( ret < 0 ){
			//if( errno == DAHUA_ECONNRESET )
			//	fprintf( stderr, "连接被对方断开\n" );
			//else
				fprintf( stderr, "接收错误\n" );
			RemoveSock( *m_conn );
			delete m_conn, m_conn = NULL;	
			return 0;								
		}else if( ret == 0 ){
			fprintf( stdout, "没有接收到任何数据，继续！\n" );
			return 0;	
		}else{
			fprintf( stdout, "\n[%d] %s\n", m_conn->GetHandle(), recv_buf );

            int iret = m_conn->Sendmsg(recv_buf, ret);
          //  printf("send %d bytes\n", ret);
            if (iret < 0)
                fprintf(stderr, "%d send failed\n", m_conn->GetHandle());	
#if 0
            int i = 0 ;
         
            for (; i < 10; i++)
            {
            int iret = m_conn->Send(recv_buf, ret);
            sleep(1);
            printf("send %d bytes\n", ret);
            if (iret < 0)
                fprintf(stderr, "%d send failed\n", m_conn->GetHandle());	
            }
#endif             
           // return 3000000;
           return 0;
		}
	}
	fprintf( stderr, "错误，不可能运行到这儿！\n" );
	return 0;			
}

int CDhtsSvr::handle_input_timeout( int handle )
{
    
    printf("handle_input_timeout %d\n", handle);

	if( m_conn != NULL && handle == m_conn->GetHandle() ){
		fprintf( stderr, "等待客户端的数据超时!\n" );
		RemoveSock( *m_conn );
		delete m_conn, m_conn = NULL;      
		return 0;                              
	}
	fprintf( stderr, "错误，不可能运行到这儿!\n" );
	return 0;
}

int CDhtsSvr::handle_close(CNetHandler *myself)
{
	delete myself;
}

#if 0
#endif 

int StartDhtsSvr()
{
	CDhtsSvr *dhts_svr = new CDhtsSvr;
	
	CSockAddrIPv4 addr( INADDR_ANY, 9000 ); 	
	if( dhts_svr->StartSvr( addr ) < 0 )
		return -1;
    fprintf(stdout, "start rudp server on 9000 port\n");
	
	// 如果输入q 字符退出
	char c = 0;
	while(c = getchar())
	{
		if ('q' == c)
		{
			printf("exit dhts svr\n");
			break;
		}

		Dahua::Infra::CThread::sleep(5 * 1000);
	}

	if (dhts_svr)
		dhts_svr->Close();
	
	Dahua::Infra::CThread::sleep(2 * 1000);

	return 0;
}
