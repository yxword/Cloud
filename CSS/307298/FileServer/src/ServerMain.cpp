#include "FileServer.h"
#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"

int main( int argc, char* argv[] )
{
    if( CNetThread::CreateThreadPool( 4, false ) < 0 )
    {
    	std::cerr << "Create thread pool faild!" << std::endl;
    	CNetThread::DestroyThreadPool();
    	return 0;
    }

    CFileServer svr( CSockAddrIPv4( "0.0.0.0", 4444 ) );
    svr.startServer();

    while( true )
        CThread::sleep(10000000);
	
    svr.stopServer();
    CNetThread::DestroyThreadPool();

    return 0;
}
