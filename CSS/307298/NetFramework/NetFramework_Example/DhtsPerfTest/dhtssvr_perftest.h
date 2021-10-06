#ifndef __INCLUDE_DHTS_SVR_PERFTEST_H
#define __INCLUDE_DHTS_SVR_PERFTEST_H

#include <cstdio>
#include <map>
#include <list>
#include "Infra/Mutex.h"
#include "Infra/Thread.h"
#include "Infra/Time.h"

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/DhtsAcceptor.h"
#include "NetFramework/DhtsStream.h"
#include "dhts_perftest.h"

using namespace Dahua::Infra;
using namespace Dahua::NetFramework;
#define FILENAME_PREFIX		"dat."

struct PerfTestConfig;

struct ConnectSession
{
	CDhtsStream *stream;
	char 	*pcdata;
	int   	 bufsize;
	FILE	*fp;	
	bool 	 seqttflag;
	int 	 total_rxbytes;
	int	 total_txbytes;
	int	 rxbytes;
	int	 txbytes;
	int	 intval;
	uint64_t lastsamplets;
	ConnectSession():stream(NULL), pcdata(NULL), bufsize(0), fp(NULL), seqttflag(false){}
};

class CDhtsSvrPerfTest: public CNetHandler
{
public:
    CDhtsSvrPerfTest();
    virtual ~CDhtsSvrPerfTest();

public:
    int StartSvr( CSockAddrIPv4 &addr,  bool seqttflag=false);
	void StopSvr();
	void Close();
	void SetPtconfig(PerfTestConfig &ptconfig);

private:
	int ProcessConnRequest();
	int ProcessData(int handle);
	ConnectSession * CreateSession(CDhtsStream *sock,  bool seqttflag=false);
	void DestroySession(ConnectSession *session);
	int DestroySessionList();
    virtual int handle_input( int handle );
    virtual int handle_input_timeout( int handle );
	 int sample(ConnectSession *session, BandwithSample *bwsample, bool clear = true);
	 void ThreadProc(  CThreadLite& arg  );

private:
	Dahua::Infra::CMutex	   m_mutex;
	std::map<int, ConnectSession*> m_mnglist;
	typedef std::map<int,ConnectSession*>::iterator ConnListIterator;
	
    CDhtsAcceptor m_listener;
	bool m_seqttflag;
	PerfTestConfig  m_ptconfig;
	CThreadLite*		m_thread;
};

#endif // __INCLUDE_DHTS_SVR_H
