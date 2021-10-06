#ifndef __INCLUDED_FILE_CLIENT_H__
#define __INCLUDED_FILE_CLIENT_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SockStream.h"
//
#include <iostream>
#include <fstream>
#include <unistd.h>

using namespace Dahua::NetFramework;
using std::cout;
using std::endl;

class CFileClient : public CNetHandler
{
public:
    CFileClient();
    ~CFileClient();

    int connectServer( const CSockAddr& remote );

    int handle_input( int handle );
    int handle_output( int handle );

    int handle_input_timeout( int handle );
    int handle_output_timeout( int handle );
    int handle_timeout( int handle );

    int handle_exception( int handle );
    int handle_close( CNetHandler* myself );

    int downloadFile();
    int uploadFile( string filename );

    void close();

private:
    CSockStream* m_stream;
    bool m_is_close;
    long m_connection_timer;
    // long m_send_timer;
};


#endif //__INCLUDED_FILE_CLIENT_H__
