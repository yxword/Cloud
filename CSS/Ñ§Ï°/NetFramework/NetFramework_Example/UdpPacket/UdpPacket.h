//
//  "$Id: udpPacket.h 7672 2010-5-17 02:28:18Z he_linqiang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-5-28        he_linqiang Create
//

#ifndef __INCLUDED__NETFRAMEWORK_UDP_PACKET_H__
#define __INCLUDED__NETFRAMEWORK_UDP_PACKET_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockPacket.h"
using namespace Dahua::NetFramework;
class CUdpPacket : public CNetHandler
{
public:
	CUdpPacket();
	virtual ~CUdpPacket();
public:
	int StartSession( CSockAddrIPv4 & addr );
	virtual int64_t handle_timeout( long id );
	virtual int handle_input( int handle );
	virtual int handle_input_timeout( int handle );
	virtual int handle_exception( int handle );
	
	virtual int handle_close(CNetHandler *myself);
private:
	CSockPacket m_packet;
	
	unsigned char		m_broadcast_mac[ETH_ALEN];
	int							m_state;
};

int StartUdpPacket();

#endif //__INCLUDED__NETFRAMEWORK_UDP_PACKET_H__
