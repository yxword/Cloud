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
#ifndef __INCLUDED_LIVE_SVR_H__
#define __INCLUDED_LIVE_SVR_H__

#include "NetFramework/MediaStreamSender.h"
#include "NetFramework/SockAcceptor.h"
#include "NetFramework/SockStream.h"
#include "list"
using namespace Dahua::NetFramework;
class CLiveSvr : public CNetHandler
{
public:
	CLiveSvr();
	~CLiveSvr();
public:
	int StartSvr( CSockAddr *local );
	int StopSvr();
public:
	virtual int handle_input( int handle );
	virtual int64_t handle_timeout( long id ); //模拟流媒体周期产生数据包
	virtual int handle_message( long from_id, MSG_TYPE type, long attach );
	
	virtual int handle_close(CNetHandler *myself);
private:
	CSockAcceptor	*m_acceptor;
	CMediaBuffer 	*m_mediabuf;
	long			m_timer;	
	std::list<CMediaStreamSender*> m_sender_list;
};

int StartLiveSvr();

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
private:
	CSockStream m_stream;
};

#endif //__INCLUDED_LIVE_SVR_H__
