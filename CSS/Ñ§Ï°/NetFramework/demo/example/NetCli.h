//
//  "$Id: NetCli.h 31672 2016-03-18 09:03:00 wang_zhihao $"
//
//  Copyright (c)1992-2016, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2016-3-18		wang_zhihao	Create
//

#ifndef __INCLUDED_NET_CLI_H__
#define __INCLUDED_NET_CLI_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SockStream.h"

#include <iostream>

using namespace Dahua::NetFramework;

class CNetCli : public CNetHandler
{
public:
	CNetCli()
		: m_is_close( false )
		, m_timer_id( 0 )
		, m_stream( NULL )
    {
    }

    ~CNetCli()
    {
    	if( m_stream != NULL )
    		delete m_stream;
    }

    int connect( const CSockAddr & remote )
    {
    	if ( m_stream != NULL )
    		return -1;

    	m_stream = new CSockStream();
    	if( m_stream->Connect( remote ) < 0 ) {
    		std::cout << "连接服务器失败！" << std::endl;
    		return -1;
    	}

    	RegisterSock( *m_stream, WRITE_MASK, 5000000 );

    	return 0;
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
    	}

    	return 0;
    }

    int handle_output( int handle )
    {
    	int ret = m_stream->GetConnectStatus();
    	if( ret == CSockStream::STATUS_NOTCONNECTED ) {
    		return 0;

    	} else if ( ret == CSockStream::STATUS_ERROR ) {
    		std::cout << "连接服务器失败！" << std::endl;
    		RemoveSock( *m_stream );
    		return -1;

    	} else {
    		std::cout << "连接服务器成功！" << std::endl;
    		UnregisterSock( *m_stream, WRITE_MASK );
    		RegisterSock( *m_stream, READ_MASK | EXCEPT_MASK );
    		m_timer_id = SetTimer( 5000000 );
    		return -1;
    	}
    }

    int64_t handle_timeout( long id )
    {
    	const char text[] = "NetFramework test text!";
    	if( m_stream->Send(text, sizeof(text)) < 0)
    		closeSession();

    	return 0;
    }

    int handle_output_timeout( int handle )
    {
		std::cout << "连接服务器超时！" << std::endl;
		RemoveSock( *m_stream );
		delete m_stream;
		m_stream = NULL;

		return -1;
    }

    int handle_exception( int handle )
    {
    	closeSession();

    	return 0;
    }

    int handle_close( CNetHandler* myself )
    {
    	delete this;

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
    long  m_timer_id;
    CSockStream* m_stream;
};

#endif //__INCLUDED_NET_CLI_H__
