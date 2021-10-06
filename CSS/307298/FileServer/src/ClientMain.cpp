#include "FileClient.h"
#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"
#include "Input.h"

using std::string;

int main( int argc, char* argv[] )
{
    // todo
    if( CNetThread::CreateThreadPool(4, false) < 0 )
    {
    	std::cerr << "Create thread pool faild!" << std::endl;
    	CNetThread::DestroyThreadPool();
    	return 0;
    }

    CFileClient* cli = new CFileClient();   ///** ÄÚ´æÐ¹Â©??
    std::cout << "connecting" << std::endl;
    cli->connectServer( CSockAddrIPv4("127.0.0.1", 7800) );

    // system( "stty -echo" );
    // system( "stty -icanon" );

    while( true ){
        string operation, filename;
        cout << "[@FileClient]# ";
        operation = CInput::inputString();
        if( operation == "downloadFile" ){
            cli->downloadFile();
        }
        else if( operation == "uploadFile" ){
            filename = CInput::inputString();
            int ret = cli->uploadFile( filename );
        }

    }

    delete cli;
    cli = NULL;
    CNetThread::DestroyThreadPool();
    return 0;
}
