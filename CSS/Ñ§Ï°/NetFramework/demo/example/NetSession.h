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

#ifndef __INCLUDED_NET_SESSION_H__
#define __INCLUDED_NET_SESSION_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SockStream.h"

#include <iostream>

#define MSG_SESSION_CLOSE 1

using namespace Dahua::NetFramework;

class CNetSession : public CNetHandler
{
public:
	CNetSession( long svrID, CSockStream* stream)
		: m_is_close( false )
		, m_svr_id( svrID )
		, m_timer_id( 0 )
		, m_stream( stream )
    {
		RegisterSock( *m_stream, READ_MASK | EXCEPT_MASK );
		m_timer_id = SetTimer( 5000000 );
    }

    ~CNetSession()
    {
    	if( m_stream != NULL )
    		delete m_stream;
    }

    int handle_input( int handle )
    {
    	//事件电平触发，不处理将一直触发
    	char buffer[4096] = {0};
    	int ret = m_stream->Recv( buffer, sizeof( buffer ) );
    	if( ret < 0 ){
    		closeSession();
    	} else if ( ret == 0 ){
    		//继续执行等待下一次事件触发
    	}
    	else {
    		std::cout << "Receive content : " << buffer << std::endl;
    		m_stream->Send( buffer, ret );
    	}

    	return 0;
    }

    int64_t handle_timeout( long id )
    {
    	char buffer[4];
    	if( m_stream->Send( buffer, 0 ) < 0)
			closeSession();

    	return 0;
    }

    int handle_exception( int handle )
    {
    	closeSession();

    	return 0;
    }

    int handle_close( CNetHandler* myself )
    {
    	Notify( m_svr_id, MSG_SESSION_CLOSE, m_stream->GetHandle() );

    	return 0;
    }
	
private:
    void closeSession()
    {
    	if( m_is_close )
    		return ;

    	RemoveSock( *m_stream );
    	DestroyTimer( m_timer_id);
    	Close();
    	m_is_close = true;
    }

private:
    bool  m_is_close;
    long  m_svr_id;
    long  m_timer_id;
    CSockStream* m_stream;
};

#endif
