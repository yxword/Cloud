//
//  "$Id: UdpSvr.h 7672 2010-5-17 02:28:18Z he_linqiang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-5-17        he_linqiang Create
//
#ifndef __INCLUDED_UDPSVR_H__
#define __INCLUDED_UDPSVR_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockDgram.h"
using namespace Dahua::NetFramework;
class CUdpSvr : public CNetHandler
{
public:
	CUdpSvr();
	virtual ~CUdpSvr();
public:
	int StartSvr( CSockAddr & addr );
	virtual int handle_input( int handle );
	virtual int handle_input_timeout( int handle );
	virtual int handle_exception( int handle );
	virtual int handle_close(CNetHandler *myself);
private:
	CSockDgram m_dgram;
};

int StartUdpSvr();

#endif //__INCLUDED_UDPSVR_H__
