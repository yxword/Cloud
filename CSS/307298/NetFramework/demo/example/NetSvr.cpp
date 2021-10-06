#include "NetSvr.h"
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
				    
    CNetSvr svr( CSockAddrIPv4( "0.0.0.0", 7800 ) );
    svr.startSvr();

    while( true )
        CThread::sleep(10000000);
	
    svr.stopSvr();
    CNetThread::DestroyThreadPool();

    return 0;
}
