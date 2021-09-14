//
//  "$Id: Multicast.h 7672 2010-5-17 02:28:18Z he_linqiang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-5-17        he_linqiang Create
//
#ifndef __INCLUDED_MULTICAST_H__
#define __INCLUDED_MULTICAST_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockDgram.h"
#include "NetFramework/SockAddrIPv4.h"

using namespace Dahua::NetFramework;
class CMulticast : public CNetHandler
{
public:
	CMulticast();
	virtual ~CMulticast();
public:
	int StartSvr( CSockAddr & addr );
	virtual int handle_input( int handle );
	virtual int64_t handle_timeout( long id );
	virtual int handle_input_timeout( int handle );
	virtual int handle_exception( int handle );
	
	virtual int handle_close(CNetHandler *myself);
private:
	CSockDgram m_dgram;
	long	m_timer_id;
	CSockAddrIPv4	m_mc_addr;
};

int StartMulticast();

#endif //__INCLUDED_MULTICAST_H__
