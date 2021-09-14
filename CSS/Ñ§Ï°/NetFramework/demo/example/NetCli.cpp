#include "NetCli.h"
#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"

int main( int argc, char* argv[] )
{
    if( CNetThread::CreateThreadPool(4, false) < 0 )
    {
    	std::cerr << "Create thread pool faild!" << std::endl;
    	CNetThread::DestroyThreadPool();
    	return 0;
    }

    for ( int i = 0; i < 1000; ++ i)
    {
    	CNetCli* cli = new CNetCli();
    	cli->connect( CSockAddrIPv4("127.0.0.1", 7800) );
    }

    while( true )
        Dahua::Infra::CThread::sleep(10000000);

    CNetThread::DestroyThreadPool();

    return 0;
}
