//
//  "$Id: MediaSender.h 7672 2010-5-17 02:28:18Z zhou_mingwei $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-7-7        zhou_mingwei Create
//
//
#ifndef __INCLUDED_LIVE_CLI_H__
#define __INCLUDED_LIVE_CLI_H__

#include "NetFramework/MediaStreamSender.h"
#include "NetFramework/SockAcceptor.h"
#include "NetFramework/SockStream.h"
#include "list"
using namespace Dahua::NetFramework;

/****************************/

class CLiveCli : public CNetHandler
{
public:
	CLiveCli();
	~CLiveCli();
public:
	int StartWatch( CSockAddr* remote );
public:
	virtual int handle_input( int handle );
	virtual int handle_output( int handle );
	
	virtual int handle_close(CNetHandler *myself);
private:
	CSockStream m_stream;
};

int StartLiveCli();

#endif //__INCLUDED_LIVE_CLI_H__
