#include "DTLSCli.h"
#include "NetFramework/SslDgram.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SslOption.h"
#include "NetFramework/SslX509.h"
#include <stdio.h>
#include <string.h>
#include "Infra/Thread.h"
int StartDtlsCli()
{
	CSslDgram dgram;
	CSockAddrIPv4 remote( "10.6.5.76", 12345 );

	CSockAddrIPv4 remote_err( "10.6.5.76", 12340 );
	
	CSslX509* x509 = new CSslX509;
	x509->SetCert("./cert/EndProduct.crt", "./cert/EndProduct.key");	
	x509->SetCA("./cert/AffiliatedCA.crt");
	x509->SetCA("./cert/RootCA.crt");
	x509->SetTrustCA("./RootCA.crt");
	dgram.SetOption( Module_X509, Type_X509_all, x509 );

	if( dgram.Connect( &remote_err) != -1 ){
		printf("Connect() succeed is wrong!\n");
		return -1;
	}
	printf("Connect() failed is right!\n");
	
	if( dgram.Connect( &remote) == -1 ){
		printf("Connect() failed!\n");
		return -1;
	}
	printf("Connect succeed!\n");
	printf("sleep 3s times!\n");
	sleep(3);
	if( dgram.Send("HelloWorld!", sizeof("HelloWorld!")) < sizeof("HelloWorld!") )
		return -1;
	printf("Send hello world succeed!\n");
	
	
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
		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}