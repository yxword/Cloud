#include "Timer.h"
#include "AsyncMsg.h"
#include "NetFramework/NetThread.h"

int main( int argc, char* argv[] )
{
    if( CNetThread::CreateThreadPool(4, false) < 0 )
    {
        std::cerr << "Create thread pool faild!" << std::endl;
        CNetThread::DestroyThreadPool();
        return 0;
    }

#if 0
    std::cout << "main thread id " << Dahua::Infra::CThread::getCurrentThreadID() << std::endl;

    CTimer timer( 1000000 );
    timer.startTimer();

    int index = 0;
    while( index ++ < 5 )
    	Dahua::Infra::CThread::sleep( 2 * 1000 );

    timer.Close();
    //std::cout << "destroy timer " << std::endl;
    //timer.destroyTimer();
    //Dahua::Infra::CThread::sleep( 5 * 1000 );
#endif
    
#if 1
    CMsgEntry entryA;
    CMsgEntry entryB(&entryA);

    entryB.start();

    int index = 0;
    while( index ++ < 5 )
    	Dahua::Infra::CThread::sleep( 2 * 1000 );
    
    entryA.Close();
    entryB.Close();
#endif

    CNetThread::DestroyThreadPool();

    return 0;
}

