#ifndef __INCLUDED_FILE_SERVER_H__
#define __INCLUDED_FILE_SERVER_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SockStream.h"
#include "NetFramework/SockAcceptor.h"
#include "NetSession.h"

#include <iostream>
using namespace Dahua::NetFramework;

class CFileServer : public CNetHandler
{
public:
    CFileServer( const CSockAddrIPv4& sockaddr );
    ~CFileServer();

    int startServer();
    void stopServer();

    int handle_input ( int handle );
    // int handle_output( int handle );

    // int handle_input_timeout ( int handle );
    // int handle_output_timeout( int handle );
    int64_t handle_timeout( long id );
    int handle_message( long from_id, MSG_TYPE type, long attach );

    // int handle_exception( int handle );
    int handle_close( CNetHandler* myself );

private:
    CSockAcceptor m_acceptor;
    CSockAddrIPv4 m_sock_addr;
    CNetSessionManager m_sessions;
};

#endif //__INCLUDED_FILE_SERVER_H__


// TODO mysql server