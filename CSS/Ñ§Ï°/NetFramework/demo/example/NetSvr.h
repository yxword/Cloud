//
//  "$Id: Timer.h 31672 2016-03-18 09:03:00 wang_zhihao $"
//
//  Copyright (c)1992-2016, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2016-3-18		wang_zhihao	Create
//

#ifndef __INCLUDED_NET_SRV_H__
#define __INCLUDED_NET_SRV_H__

#include "NetSession.h"
#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SockAcceptor.h"
#include "NetFramework/SockStream.h"
#include "Infra/Mutex.h"
#include <iostream>
#include <map>

using namespace Dahua::NetFramework;
using namespace Dahua::Infra;

class CSessionManager
{
public:
	CSessionManager()
	{

	}

	~CSessionManager()
	{
	      clear();
	}

	CNetSession* createFactory( long svrID, CSockStream* stream )
	{
		CNetSession* session = new CNetSession( svrID, stream );

		m_mutex.enter();
		m_sessions.insert( std::make_pair( stream->GetHandle(), session ) );
		m_mutex.leave();

		return session;
	}

	void remove( int handle )
	{
		m_mutex.enter();
		std::map<int, CNetSession*>::iterator it = m_sessions.find( handle );
		if( it != m_sessions.end() ){
			delete it->second;
			m_sessions.erase(it);
		}
		m_mutex.leave();
	}

	void clear()
	{
		m_mutex.enter();
    	for( std::map<int, CNetSession*>::iterator it = m_sessions.begin(); it != m_sessions.end(); it ++)
    		delete it->second;
    	m_sessions.clear();
    	m_mutex.leave();
	}

	std::size_t size()
	{
	    return m_sessions.size();
	}

private:
    CMutex m_mutex;
    std::map<int , CNetSession*> m_sessions;
};

class CNetSvr : public CNetHandler
{
public:
	CNetSvr(const CSockAddrIPv4& sockaddr)
		: m_sockaddr(sockaddr)
    {
    }

    ~CNetSvr()
    {
    	stopSvr();
    }

    int startSvr()
    {
    	if( m_acceptor.Open( m_sockaddr ) < 0 )
    	{
    		std::cerr << "监听端口 " << m_sockaddr.GetPort() << "失败！" << std::endl;
    		return -1;
    	}

    	RegisterSock( m_acceptor, READ_MASK );
    	SetTimer( 10000000 );

    	return 0;
    }

    void stopSvr()
    {
    	m_acceptor.Close();
    }

    virtual int handle_input( int handle )
    {
    	if( m_acceptor.GetHandle() == handle ) {
    		CSockAddrIPv4 remote_addr;
    		CSockStream* stream = m_acceptor.Accept( &remote_addr );

    		if( stream != NULL ) {
    			RemoveSock( *stream );
    			m_sessions.createFactory( GetID(), stream);

    			char ip[32] = {0};
    			std::cout << "New session ip " << remote_addr.GetIpStr( ip, sizeof(ip) )
    					  << ", port " << remote_addr.GetPort()
    					  << std::endl;
    		}
    	} else {

    	}

    	return 0;
    }

    int handle_close( CNetHandler* myself )
    {
    	stopSvr();
    	return 0;
    }

    int64_t handle_timeout( long id )
    {
        std::cout << "连接客户端数量：" << m_sessions.size() << std::endl; 
        return 0;
    }

    virtual int handle_message( long from_id, MSG_TYPE type, long attach )
    {
    	switch( type )
    	{
    		case MSG_SESSION_CLOSE:
    			m_sessions.remove( (int)attach );
    			break;
    		default:
    			std::cout << "无法处理消息 " << type << std::endl;
    			break;
    	}

    	return 0;
    }

private:
    CSockAddrIPv4 m_sockaddr;
    CSockAcceptor m_acceptor;

    CSessionManager m_sessions;
};

#endif
