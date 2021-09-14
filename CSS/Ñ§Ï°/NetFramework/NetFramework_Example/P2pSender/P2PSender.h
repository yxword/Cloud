//
//  "$Id: P2PSender.h 7672 2010-5-17 02:28:18Z zhou_mingwei $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-7-7        zhou_mingwei Create
//
//

#include "NetFramework/StreamSender.h"
#include "NetFramework/SockStream.h"
#include "NetFramework/SockAcceptor.h"

#ifndef __INCLUDE_P2P_SENDER__
#define __INCLUDE_P2P_SENDER__
using namespace Dahua::NetFramework;
class CP2PSender : public CNetHandler
{
public:
	CP2PSender();
	~CP2PSender();
public:
	int StartSender( CSockAddr &local );
	int StopSender();
public:
	virtual int handle_input( int handle );
	virtual int64_t  handle_timeout( long id );
	virtual int handle_message( long from_id, MSG_TYPE type, long attach );
	virtual int handle_output( int handle );
	
	virtual int handle_close(CNetHandler *myself);
private:
	CStreamSender *m_sender;
	CSockAcceptor m_acceptor;
	long		  m_timer;
	CSockStream   *m_stream;
};

int StartP2pSenderTest();

#endif //__INCLUDE_P2P_SENDER__
