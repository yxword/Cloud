#include "FileServer.h"

CFileServer::CFileServer( const CSockAddrIPv4& sockaddr )
    : m_sock_addr( sockaddr )
{

}

CFileServer::~CFileServer()
{
    stopServer();
}

int CFileServer::startServer()
{
    if( m_acceptor.Open( m_sock_addr ) < 0 ){
        cout << "listen failed" << endl;
        return -1;
    }
    RegisterSock( m_acceptor, READ_MASK );
    SetTimer( 30000000 ); // 30s

    return 0;
}

void CFileServer::stopServer()
{
    RemoveSock( m_acceptor );
    m_acceptor.Close();
}

int CFileServer::handle_input( int handle )
{
    if( m_acceptor.GetHandle() == handle ){
        CSockAddrIPv4 client_addr;
        CSockStream* stream = m_acceptor.Accept( &client_addr );
        if( stream == NULL ) return 0;

        RemoveSock( *stream ); // remove register information in the framework first
        m_sessions.createFactory( GetID(), stream );
        char ip[32] = {0};
        cout << "New Client ip : " << client_addr.GetIpStr( ip, sizeof( ip ) )
             << ", port " << client_addr.GetPort() << endl;
    }
    else{
        // TODO
    }

    return 0;
}

// int CFileServer::handle_output( int handle )
// {

// }

// int CFileServer::handle_input_timeout ( int handle )
// {

// }

// int CFileServer::handle_output_timeout( int handle )
// {

// }

int64_t CFileServer::handle_timeout( long id )
{
    cout << "session size: " << m_sessions.size() << endl;
    return 0;
}

int CFileServer::handle_message( long from_id, MSG_TYPE type, long attach )
{
    switch( type )
    {
        case MSG_SESSION_CLOSE:
            m_sessions.remove( (int)attach );
            break;
        default:
            std::cout << "can't handle this message " << type << std::endl;
            break;
    }

    return 0;
}

// int CFileServer::handle_exception( int handle )
// {

// }

int CFileServer::handle_close( CNetHandler* myself )
{
    // stopServer();
    return 0;
}

