#ifndef __INCLUDED_FILE_CLIENT_H__
#define __INCLUDED_FILE_CLIENT_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SockStream.h"
#include "Infra/Thread.h"
#include "StringBuffer.h"
//
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <errno.h>

// #define FILE_HEAD_SIZE 4096

using namespace Dahua::NetFramework;
using namespace Dahua::Infra;
using namespace std;


class CFileClient : public CNetHandler
{
public:
    CFileClient();
    ~CFileClient();

    int connectServer( const CSockAddr& remote );

    int handle_input ( int handle );
    int handle_output( int handle );

    // int handle_input_timeout ( int handle );
    int handle_output_timeout( int handle );
    int64_t handle_timeout( long id );

    int handle_exception( int handle );
    int handle_close( CNetHandler* myself );

    void uploadFile  ( string filename );
    void downloadFile( string filename );
    void download( string filename, uint64_t file_size );

    bool isClose() { return m_is_close; }
    void close();

private:
    CSockStream* m_stream;
    bool m_is_close;
    long m_connection_timer;
    // long m_send_timer;
};


#endif //__INCLUDED_FILE_CLIENT_H__
