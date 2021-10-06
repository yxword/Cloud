#ifndef __INCLUDED_NET_SESSION_H__
#define __INCLUDED_NET_SESSION_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SockStream.h"
#include "Infra/Thread.h"
#include "Infra/Mutex.h"

#include <map>
#include <iostream>
#include <fstream>
#include <errno.h>

#include "StringBuffer.h"

#define MSG_SESSION_CLOSE 1

using namespace Dahua::NetFramework;
using namespace Dahua::Infra;
using namespace std;

class CNetSession : public CNetHandler
{
public:
	CNetSession( long svrID, CSockStream* stream);
    ~CNetSession();

    int handle_input( int handle );
    int64_t handle_timeout( long id );
    int handle_exception( int handle );
    int handle_close( CNetHandler* myself );
	
private:
    void closeSession();
	void handleUploadRequest( string filename, uint64_t file_size );
	void handleDownloadRequest( string filename );
	
private:
    bool  m_is_close;
    long  m_svr_id;
    long  m_timer_id;
    CSockStream* m_stream;
};


// Session Manager
class CNetSessionManager
{
public:
    CNetSessionManager();
	~CNetSessionManager();

	CNetSession* createFactory( long svrID, CSockStream* stream );
	void remove( int handle );
	void clear();
	uint64_t size() { return m_sessions.size(); }
private:
	CMutex m_mutex;
	map<int, CNetSession*> m_sessions;
};

#endif
