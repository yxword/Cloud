#include <stdio.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>

#include "dhtscli_perftest.h"

CDhtsCliPerfTest::CDhtsCliPerfTest():m_inited(false),m_sema(0)
{
}

CDhtsCliPerfTest::~CDhtsCliPerfTest()
{
}

void CDhtsCliPerfTest::SetPtconfig(PerfTestConfig &ptconfig)
{
	m_ptconfig = ptconfig;
}

int CDhtsCliPerfTest::StartSession()
{
	if (m_inited)
		return 0;
	
	if(setupClients() < 0)
	{
		return -1;
	}

	m_inited = true;
	
	return 0;
}

int CDhtsCliPerfTest::RestartSession()
{
	if (!m_inited)
		return -1;
	
	SessionListIt pit;
	ClientSession* session;

	m_mutex.enter();
	for (pit = m_sessionlist.begin(); pit!=m_sessionlist.end();)
	{
		if ((session = pit->second)!=NULL)
		{

			RegisterSock(session->stream, WRITE_MASK);
	#if 0
			if (RegisterSock(session->stream, WRITE_MASK) < 0)
			{
				m_sessionlist.erase(pit++);
				RemoveSock(session->stream);
				deinitSession(session);
				delete session,session=NULL;
				continue;
			}			
	#endif 
		}

		pit++;
	}
	m_mutex.leave();

	return 0;
}

void CDhtsCliPerfTest::Close()
{
	if (m_inited)
	{
		m_inited = false;

		destroyClients();
		CNetHandler::Close();

		// wait handle_close 
		m_sema.pend();
	}
}

int CDhtsCliPerfTest::setupClients()
{
	int i;
	ClientSession *session = NULL;
	
	for (i = 0; i < m_ptconfig.nchans; i++)
	{
		session = new ClientSession;
		
		if (initSession(session, m_ptconfig.seqtimetest) < 0)
		{
			delete session,session=NULL;
			continue;
		}
		RegisterSock(session->stream, WRITE_MASK);
		
		printf("setup client session %d success\n", session->stream.GetHandle());

		m_mutex.enter();
		m_sessionlist[session->stream.GetHandle()] = session;		
		m_mutex.leave();
	}

	// no client session has been setup successfully, return failed
	if (m_sessionlist.size() == 0)
		return -1;
	
	return 0;
}

void CDhtsCliPerfTest::destroyClients()
{
	ClientSession *session;
	SessionListIt pit;
	
	m_mutex.enter();

	for (pit = m_sessionlist.begin(); pit!= m_sessionlist.end();)
	{
		session = pit->second;
		m_sessionlist.erase(pit++);

		if (session)
		{
			RemoveSock(session->stream);
			deinitSession(session);
			delete session,session=NULL;
		}
	}
	
	m_mutex.leave();
}

int CDhtsCliPerfTest::initSession(ClientSession *session, bool seqtime_test)
{
	char *bufptr = NULL;
	CSockAddrIPv4 servaddr(m_ptconfig.addr);

	if (session->stream.Connect(servaddr) < 0)
	{
		printf("Connect to serv %s:%d failed\n", 
					inet_ntoa(m_ptconfig.addr.sin_addr), ntohs(m_ptconfig.addr.sin_port));
		return -1;
	}

	session->seqtime_test = seqtime_test;

	session->bufsize = calcTxbufsize();
	bufptr = new char[session->bufsize];
	if (!bufptr)
	{
		session->bufsize = 0;
		session->stream.Close();
		return -1;
	}
	session->bufptr = bufptr;
	session->total_txbytes = 0;
	return 0;
}

void CDhtsCliPerfTest::deinitSession(ClientSession *session)
{
	printf("conn<%d> send total %d bytes data\n", session->stream.GetHandle(), session->total_txbytes);
	if (session->bufptr)
	{
		delete [](session->bufptr);
		session->bufptr = NULL;
		session->bufsize = 0;
	}

	session->stream.Close();
}


int CDhtsCliPerfTest::calcTxbufsize()
{
	return (m_ptconfig.bandwidth >> 3) / m_ptconfig.fps;
}


int CDhtsCliPerfTest::getSeqTime(char * bufptr,int len)
{	
	int cnt;

	time_t ticks = time(NULL);
	cnt = snprintf(bufptr, len-1, "%s\n", ctime(&ticks));
	bufptr[cnt] = 0;
	return cnt+1;
}

int CDhtsCliPerfTest::doSend(ClientSession *session)
{
	int len = 0;
	
	if (session->seqtime_test)
	{
		int cnt = getSeqTime(session->bufptr, session->bufsize);
		printf("<%d>: %s", session->stream.GetHandle(), session->bufptr);
		len = session->stream.Send(session->bufptr, cnt); 
	}
	else 
	{
		len = session->stream.Send(session->bufptr, session->bufsize);
	}

	return len;
}

int CDhtsCliPerfTest::handle_input(int handle)
{
	return 0;
}

int CDhtsCliPerfTest::handle_input_timeout(int handle)
{
    printf("%d handle_input_timeout\n", handle);
    return -1;
}

int CDhtsCliPerfTest::handle_output( int handle )
{
	ClientSession *session;

	m_mutex.enter();
	std::map<int, ClientSession*>::iterator it = m_sessionlist.find(handle);
	if (it == m_sessionlist.end())
	{
		m_mutex.leave();
		printf("cannot find client session with handle %d\n", handle);
		return -1;
	}

	if ((session = it->second)!=NULL)
	{
		int ret = session->stream.GetConnectStatus();
		if( ret == CDhtsStream::STATUS_NOTCONNECTED ){
			m_mutex.leave();
			printf( "socket handle:%d connecting...!\n", handle);
			return 0;						
		}else if( ret == CDhtsStream::STATUS_ERROR ){
			m_sessionlist.erase(it);
			m_mutex.leave();
			printf( "connect failed, handle:%d\n", handle );
			RemoveSock(session->stream);
			deinitSession(session);
			delete session, session=NULL;
			return -1;						
		}else{
			UnregisterSock( session->stream, WRITE_MASK ); 
		
			ret = doSend(session);
			if ( ret < 0)
			{
				m_sessionlist.erase(it);
				m_mutex.leave();
				RemoveSock(session->stream);
				deinitSession(session);
				delete session, session=NULL;
				return -1;

			}
			session->total_txbytes += ret;
			//printf("send %d bytes to svr\n", ret);		
		}

	}
	m_mutex.leave();
	return 0;
}

int CDhtsCliPerfTest::handle_output_timeout( int handle )
{
	ClientSession *session;

	printf( "handle_output_timeout, handle:%d\n", handle);
	
	m_mutex.enter();
	std::map<int, ClientSession*>::iterator it = m_sessionlist.find(handle);
	if (it == m_sessionlist.end())
	{
		m_mutex.leave();
		printf("cannot find client handle %d\n", handle);
		return -1;
	}

	if ((session = it->second) != NULL)
	{
		m_sessionlist.erase(it);
		m_mutex.leave();
		RemoveSock(session->stream);
		deinitSession(session);
		delete session, session=NULL;
	}
	else
		m_mutex.leave();
	
	return -1;	
}

int CDhtsCliPerfTest::handle_exception( int handle )
{
	ClientSession *session;
	printf( "handle_exception, handle%d\n", handle);

	m_mutex.enter();
	std::map<int, ClientSession*>::iterator it = m_sessionlist.find(handle);
	if (it == m_sessionlist.end())
	{
		m_mutex.leave();
		printf("cannot find client handle %d\n", handle);
		return -1;
	}

	if ((session = it->second) != NULL)
	{
		m_sessionlist.erase(it);
		m_mutex.leave();
		RemoveSock(session->stream);
		deinitSession(session);
		delete session, session=NULL;
	}
	else
		m_mutex.leave();


	return -1;	
}

int CDhtsCliPerfTest::handle_close(CNetHandler* myself)
{
	m_sema.post();
}


