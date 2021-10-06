#include "DTLSSvr.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SslOption.h"
#include "NetFramework/SslX509.h"
#include <stdio.h>
#include <string.h>
#include "Infra/Thread.h"
CDTLSSvr::CDTLSSvr()
{
#ifndef SSL_DGRAM_SYNCHRONISM
	accepted = 0;
#endif
}

CDTLSSvr::~CDTLSSvr()
{
}

int CDTLSSvr::StartSvr( CSockAddr & addr )
{	
	CSslX509* x509 = new CSslX509;
	x509->SetCert("./cert/EndProduct.crt", "./cert/EndProduct.key");	
	x509->SetCA("./cert/AffiliatedCA.crt");
	x509->SetCA("./cert/RootCA.crt");
	
	x509->SetTrustCA("./cert/RootCA.crt");
	m_dgram.SetOption( Module_X509, Type_X509_all, x509 );
#ifdef SSL_DGRAM_SYNCHRONISM
	if( m_dgram.Accept( &addr ) < 0 ){
		printf("Accept() failed!\n");
		return -1;
	}else
		printf("Accept() succeed!\n");
#else
	if( m_dgram.Open( &addr ) < 0 ){
		printf("Open() failed!\n");
		return -1;
	}else
		printf("Open() succeed!\n");
#endif 
	RegisterSock( m_dgram, READ_MASK, 5000000 );
	return 0;
}

int CDTLSSvr::handle_input( int handle )
{
	char recv_buf[1024];
	int ret = 0;
	if( handle == m_dgram.GetHandle() ){
#ifndef SSL_DGRAM_SYNCHRONISM
		if( !accepted & m_dgram.AcceptX() < 0 ){
			fprintf( stderr, "AcceptX succeed!\n" );
			RemoveSock( m_dgram );
			return -1;
		}else{
			fprintf( stderr, "AcceptX failed!\n" );
			accepted = 1;
		}
#endif
		ret = m_dgram.Recv( recv_buf, sizeof(recv_buf) );
		if( ret < 0 ){
			fprintf( stderr, "接收udp数据报失败!\n" );
			RemoveSock( m_dgram );
			return -1;
		}else if( ret > 0 ){
			fprintf( stdout, "接收到来自对端的数据: %s\n", recv_buf );
			return 0;		//维持原来超时。
		}
		return 0;
	}
	return 0;
}

int CDTLSSvr::handle_exception( int handle )
{
	if( handle == m_dgram.GetHandle() ){
		fprintf( stderr, "udp socket异常!\n" );
		RemoveSock( m_dgram );
		return -1;
	}
	return -1;
}

int CDTLSSvr::handle_input_timeout( int handle )
{
	if( handle == m_dgram.GetHandle() ){
		fprintf( stderr, "udp socket超时!\n" );
		RemoveSock( m_dgram );
		return -1;
	}
	return -1;
}

int CDTLSSvr::handle_close(CNetHandler *myself)
{
	delete myself;
}

#if 0
#endif 

int StartDtlsSvr()
{
	CDTLSSvr *dtlssvr = new CDTLSSvr;		
	CSockAddrIPv4 addr( INADDR_ANY, 12345 ); 	
	if( dtlssvr->StartSvr( addr ) < 0 )
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
			printf("exit \n");
			break;
		}

		Dahua::Infra::CThread::sleep(1 * 1000);
	}
	
	if (dtlssvr)
		dtlssvr->Close();
		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}