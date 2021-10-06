

#if defined(TCP_SVR)
#include "TcpSvr.h"
#elif defined(TCP_CLI)
#include "TcpCli.h"
#elif defined(UDP_SVR)
#include "UdpSvr.h"
#elif defined(UDP_CLI)
#include "UdpCli.h"
#elif defined(MULTICAST)
#include "Multicast.h"
#elif defined(PACKET)
#include "ping.h"
#elif defined(UDPPACKET)
#include "UdpPacket.h"
#elif defined(SSL_SVR)
#include "SslSvr.h"
#elif defined(SSL_CLI)
#include "SslCli.h"
#elif defined(UDP_SVR_SSL)
#include "DTLSSvr.h"
#elif defined(UDP_CLI_SSL)
#include "DTLSCli.h"
#elif defined(ECHO_SERVER)
#include "EchoSvr.h"
#elif defined(ECHO_CLIENT)
#include "EchoCli.h"
#elif defined(TIMER)
#include "Timer.h"
#elif defined(MESSAGE)
#include "Message.h"
#elif defined(WRITE_EVENT)
#include "WriteEvent.h"
#elif defined(LIVE_SVR)
#include "LiveSvr.h"
#elif defined(LIVE_CLI)
#include "LiveCli.h"
#elif defined(P2P_SENDER)
#include "P2PSender.h"
#elif defined(STR_PARSER)
#include "StringParser.h"
#elif defined(FILE_COPY)
#include "FileCopy.h"
#elif defined(PRIVATE_POOL)
#include "PrivatePool.h"
#elif defined(DHTS_SVR)
#include "DhtsSvr.h"
#elif defined(DHTS_CLIENT)
#include "DhtsCli.h"
#elif defined(SOCK_PROTECT_SVR)
#include "SockProtSvr.h"
#elif defined(SOCK_PROTECT_CLI)
#include "SockProtCli.h"
#elif defined(NB_SSL_SVR)
#include "NbSslSvr.h"
#elif defined(NB_SSL_CLI)
#include "NbSslCli.h"
#elif defined(SSL_SVR_DEMO)
#include "SslSvrDemo.h"
#elif defined(SSL_CLI_DEMO)
#include "SslCliDemo.h"
#elif defined(HTTP_SVR)
#include "HttpSvr.h"
#endif 

#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"
#include "NetFramework/SockDgram.h"
#include "NetFramework/SslDgram.h"

#include <unistd.h>


using namespace Dahua::NetFramework;


int main( int argc, char **argv )
{
	if( CNetThread::CreateThreadPool( 4, false ) < 0 ){
		fprintf( stderr, "创建线程池失败!\n" );
		CNetThread::DestroyThreadPool();
		return -1;
	}

#if defined(DHTS_SVR)
	StartDhtsSvr();
#elif defined(DHTS_CLIENT)
	StartDhtsCli();
#elif defined(TCP_SVR)
	StartTcpSvr();
#elif defined(TCP_CLI)
	StartTcpCli();
#elif defined(UDP_SVR)
	StartUdpSvr();
#elif defined(UDP_CLI)
	StartUdpCli();
#elif defined(UDP_SVR_SSL)
	StartDtlsSvr();
#elif defined(UDP_CLI_SSL)
	StartDtlsCli();
#elif defined(MULTICAST)
	StartMulticast();
#elif defined(PACKET)
	StartCpingTest();
#elif defined(UDPPACKET)
	StartUdpPacket();
#elif defined(SSL_SVR)
	StartSslSvr(argc, argv);
#elif defined(SSL_CLI)
	StartSslCli(argc, argv);
#elif defined(ECHO_SERVER)
	StartEchoSvr();
#elif defined(ECHO_CLIENT)
	StartEchoCli(argc, argv);
#elif defined(TIMER)
	StartTimerTest();
#elif defined(MESSAGE)
	StartMessageTest();
#elif defined(WRITE_EVENT)
	StartWriteEvent();
#elif defined(LIVE_SVR)
	StartLiveSvr();
#elif defined(LIVE_CLI)
	StartLiveCli();
#elif defined(P2P_SENDER)
	StartP2pSenderTest();
#elif defined(STR_PARSER)
	StartStringParserTest();
#elif defined(FILE_COPY)
	StartFileCopyTest();
#elif defined(PRIVATE_POOL)
	StartPrivatePoolTest();
#elif defined(SOCK_PROTECT_SVR)
	StartSockProtSvr();
#elif defined(SOCK_PROTECT_CLI)
	StartSockProtCli();
#elif defined(NB_SSL_SVR)
	StartNbSslSvr(argc, argv);
#elif defined(NB_SSL_CLI)
	StartNbSslCli(argc, argv);
#elif defined(SSL_SVR_DEMO)
	StartSslSvrDemo(argc, argv);
#elif defined(SSL_CLI_DEMO)
	StartSslCliDemo(argc, argv);
#elif defined(HTTP_SVR)
	StartHttpSvr(argc, argv);
#endif

	CNetThread::DestroyThreadPool();
	return 0;
}
