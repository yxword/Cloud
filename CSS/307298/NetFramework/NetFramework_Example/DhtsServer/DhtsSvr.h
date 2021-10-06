
#ifndef __INCLUDE_DHTS_SVR_H
#define __INCLUDE_DHTS_SVR_H

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/DhtsAcceptor.h"
#include "NetFramework/DhtsStream.h"

using namespace Dahua::NetFramework;

class CDhtsSvr: public CNetHandler
{
public:
    CDhtsSvr();
    virtual ~CDhtsSvr();

public:
    int StartSvr( CSockAddrIPv4 &addr);

public:
    virtual int handle_input( int handle );
    virtual int handle_input_timeout( int handle );
	virtual int handle_close(CNetHandler *myself);
private:
    CDhtsStream *m_conn;
    CDhtsAcceptor m_acceptor;
};

int StartDhtsSvr();

#endif // __INCLUDE_DHTS_SVR_H
