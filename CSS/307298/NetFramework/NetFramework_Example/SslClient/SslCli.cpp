#include "SslX509.h"
#include "SslCli.h"
#include "SslOption.h"

#include <stdio.h>
#include <string.h>

#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"
#include "Infra/Time.h"

#ifdef _USE_OPENSSL

CSslCli::CSslCli()
{
}

CSslCli::~CSslCli()
{
	RemoveSock( m_stream );
}

int CSslCli::StartSession( CSockAddr & addr, int session )
{
	//这个判断没有必要，Connect方法里会判断，写在这里是为了帮助更好地理解地址对象。
	if( addr.GetType() != CSockAddr::SOCKADDR_TYPE_IPV4 ){
		fprintf( stderr, "地址类型错误！\n");
		return -1;
	}
	//设置密钥文件的路径
	//char* path = "./../ssl_pem";
	//m_stream.SetPemPath( path );
	
	CSslX509* x509 = new CSslX509;
	if( x509->SetTrustCA("./cert/RootCA.crt") ){
		delete x509, x509=NULL;
		printf("set certificate failed!\n");
		return -1;
	}else{
		//printf("set certificate succeed!\n");
		x509->SetCert("./cert/EndProduct.crt", "./cert/EndProduct.key");
		//x509->SetCA("./cert/AffiliatedCA.crt");
		x509->SetCA("./cert/RootCA.crt");
	}
	m_stream.SetOption( Module_X509, Type_X509_all, x509 );
	if( session & Type_Session_id ){
		m_stream.SetOption( Module_Session, Type_Session_id, NULL );
		//printf("set Session id succeed!\n");
	}else if( session & Type_Session_ticket ){
		m_stream.SetOption( Module_Session, Type_Session_ticket, NULL );
		//printf("set Session ticket succeed!\n");
	}
	printf("ssl connect start\n");
	//Connect返回-1则直接失败，但返回大于等于0表示成功。
	if( m_stream.Connect( addr ) < 0 ){
		printf( "ssl connect failed\n" );
		return -1;
	}
	printf("ssl connect success\n");
	//注册写事件
	if( do_send() < 0 )
		RegisterSock( m_stream, WRITE_MASK, 5000000 );
	return 0;
}
int CSslCli::handle_input( int handle )
{
	printf( "run in handle input!\n" );
	return 0;
}
int CSslCli::handle_output( int handle )
{
	//如果m_stream在发送数据的过程中因为发不出去注册了写事件。
	printf( "run in handle_output!\n" );
	if( handle == m_stream.GetHandle() ){		
		printf( "goto send data!\n" );
		do_send();
		UnregisterSock( m_stream, WRITE_MASK );
	}
	return 0;
}

char https_str[]="GET / HTTP/1.1"
"Accept: application/x-ms-application, image/jpeg, application/xaml+xml, image/gif, image/pjpeg, application/x-ms-xbap, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/msword, */*"
"Accept-Language: zh-CN"
"User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; WOW64; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; .NET4.0C; .NET4.0E; InfoPath.2)"
"Accept-Encoding: gzip, deflate"
"If-Modified-Since: Thu, 17 Sep 2015 08:21:41 GMT"
"If-None-Match: \"1442478101:1e8d3\""
"Host: 172.29.2.171"
"Host: 172.29.2.171"
"Connection: Keep-Alive"
"Connection: Keep-Alive"
"Cookie: DHLangCookie30=English; DhWebCookie=%7B%22username%22%3A%22admin%22%2C%22pswd%22%3A%22%22%2C%22talktype%22%3A1%2C%22logintype%22%3A0%7D; pwdNoTip=1;e=768";

int CSslCli::do_send()
{
	char send_buf[1024];
	//strncpy( send_buf, "hello world!\n", sizeof(send_buf) );
	strncpy( send_buf, https_str, sizeof(send_buf) );
	int ret = m_stream.Send( send_buf, strlen(send_buf) );
	if( ret < 0 )
		fprintf( stderr, "send data error!\n" );
	else{
		if( ret < strlen(send_buf) )
			ret = -1;
		printf( "send %d bytes!\n", ret );
		printf("%s\n", send_buf);
	}
	return ret;
}

int CSslCli::handle_output_timeout( int handle )
{
	printf( "timeout!\n" );
	if( handle == m_stream.GetHandle() ){
		printf( "link is timeout!\n" );
		RemoveSock( m_stream );
	}
	return -1;			//超时机制一律不再保留。
}

int CSslCli::handle_exception( int handle )
{
	if( handle == m_stream.GetHandle() ){
		printf( "连接发生异常!\n" );
		RemoveSock( m_stream );
	}
	return -1;
}


int CSslCli::handle_close(CNetHandler *myself)
{
	delete myself;
	return 0;
}

#if 0
#endif 

int doSslCliTest(int argc, char **argv)
{
	int total = 100;
	int session = 0;
	if( argc >= 2 )
		total = atoi(*(argv+1));
	if( argc >= 3 )
		session = atoi(*(argv+2));
	
#define CYCLE_TIMES 10 
	unsigned long tm[CYCLE_TIMES+1];
	CSslCli **sslCliArray = new CSslCli *[total *CYCLE_TIMES];
	for (int j = 0; j < CYCLE_TIMES * total; j++)
	{
		sslCliArray[j] = NULL;
	}

	for(int j  = 0; j < CYCLE_TIMES; j ++ ){
		tm[j] = Dahua::Infra::CTime::getCurrentMicroSecond();
		for( int i = 0; i < total; i ++ ){
			int offset = j * CYCLE_TIMES + i;
			sslCliArray[offset] = new CSslCli;
			CSockAddrIPv4 addr( "127.0.0.1", 9999 );
			if ( sslCliArray[offset]->StartSession( addr, session ) < 0 )
				return -1;

            sleep(60);
            printf("test exit\n");
		}
		tm[j] = Dahua::Infra::CTime::getCurrentMicroSecond() - tm[j];
	}
	tm[CYCLE_TIMES] = 0;
	for( int j = 0; j < CYCLE_TIMES; j++ ){
		printf(" %d of %d times connection finished! cost %ld microseconds \n", j, total, tm[j] );
		tm[CYCLE_TIMES] +=  tm[j];
	}
	tm[CYCLE_TIMES] /= CYCLE_TIMES;
	printf("average %ld \n", tm[CYCLE_TIMES] );
	
	// 如果输入q 字符退出
	char c = 0;
	while((c = getchar()))
	{
		if ('q' == c)
		{
			printf("exit \n");
			break;
		}

		Dahua::Infra::CThread::sleep(1 * 1000);
	}
	
	CSslCli *pit;
	for (int j = 0; j < CYCLE_TIMES; j++)
	{
		for( int i = 0; i < total; i ++ ){
			pit = sslCliArray[j * CYCLE_TIMES + i];
			pit->Close();
		}
	}
	delete []sslCliArray,  sslCliArray= NULL;
		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}

int StartSslCli(int argc, char **argv)
{
	CSslCli *sslcli = new CSslCli;		
	CSockAddrIPv4 addr( INADDR_ANY, 9999 ); 	
	if( sslcli->StartSession( addr, 0) < 0 )
	{	
		printf("start ssl cli failed\n");
		return -1;
	}
	
	// 如果输入"quit" 字符退出
	char buf[32];
	while (fgets(buf, sizeof(buf), stdin))
	{
		if (!strncmp(buf, "quit", 4))
		{
			printf("exit \n");
			break;
		}

		Dahua::Infra::CThread::sleep(1 * 1000);
	}
	
	if (sslcli)
	{
		//sslcli->StopSvr();
		sslcli->Close();
	}	
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}

#endif

