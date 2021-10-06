#include "FileClient.h"
#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"
#include "Input.h"

// using std::string;

int main( int argc, char* argv[] )
{
    // todo
    if( CNetThread::CreateThreadPool(4, false) < 0 )
    {
    	std::cerr << "Create thread pool faild!" << std::endl;
    	CNetThread::DestroyThreadPool();
    	return 0;
    }

    CFileClient* cli = new CFileClient();   
    std::cout << "connecting" << std::endl;
    cli->connectServer( CSockAddrIPv4("127.0.0.1", 4444) );

    // system( "stty -echo" );
    // system( "stty -icanon" );

    while( true ){
        if( cli->isClose() ) {
            cout << "connection close" << endl;
            break;
        }
        string operation, filename;
        cout << "[@FileClient]# ";
        operation = CInput::inputString();
        if( operation == "q" || operation == "quit" ) break;

        filename = CInput::inputString();
        if( operation == "download" ){
            cli->downloadFile( filename );
        }
        else if( operation == "upload" ){
            cli->uploadFile( filename );
        }
        Dahua::Infra::CThread::sleep( 5 * 1000 );
    }

    cli->close();
    delete cli;
    cli = NULL;
    cout <<"test" << endl;
    CNetThread::DestroyThreadPool();
    cout <<"test" << endl;
    return 0;
}
