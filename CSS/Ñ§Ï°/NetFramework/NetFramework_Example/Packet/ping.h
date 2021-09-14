//
//  "$Id: ping.h 7672 2010-5-17 02:28:18Z he_linqiang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-5-28        he_linqiang Create
//

#ifndef __INCLUDED_NETFRAMEWORK_PING_H
#define __INCLUDED_NETFRAMEWORK_PING_H

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockPacket.h"

#include <netinet/ip_icmp.h>
using namespace Dahua::NetFramework;
class CPing : public CNetHandler
{
public:
	CPing();
	virtual ~CPing();
public:
	int StartPing( CSockAddrIPv4& addr );
	virtual int64_t handle_timeout( long id );
	virtual int handle_input( int handle );
	virtual int handle_input_timeout( int handle );
	virtual int handle_exception( int handle );
	
	virtual int handle_close(CNetHandler *myself);
private:
	struct icmphdr	m_icmphdr;
	CSockPacket 		m_packet;
	
	unsigned char		m_broadcast_mac[ETH_ALEN];
	int							m_state;
	uint16_t				m_seq;
};

int StartCpingTest();

#endif //__INCLUDED_NETFRAMEWORK_PING_H
