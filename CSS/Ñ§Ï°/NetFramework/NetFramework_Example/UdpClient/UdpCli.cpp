
#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"
#include "NetFramework/SockDgram.h"
#include "NetFramework/SslDgram.h"

using namespace Dahua::NetFramework;

int StartUdpCli()
{
	CSockAddrIPv4 local_addr( INADDR_ANY, 12347 );
	CSockDgram sock;
	sock.Open( &local_addr );
	
	CSockAddrIPv4 remote_addr( INADDR_ANY, 12346 );
	sock.SetRemote( &remote_addr);
	sock.Send("hello world!", sizeof("hello world!") );
	
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
		
	Infra::CThread::sleep(2 * 1000);
	
	return 0;
}