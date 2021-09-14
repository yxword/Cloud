#include "dhtssvr_perftest.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "Dhts/Dhts.h"

#define RX_MAX_BUFSIZE	(128 * 1024)
#define FNAME_PRFIX	"dat."

CDhtsSvrPerfTest::CDhtsSvrPerfTest():m_thread(NULL)
{
}

CDhtsSvrPerfTest::~CDhtsSvrPerfTest()
{
}

void CDhtsSvrPerfTest::SetPtconfig(PerfTestConfig &ptconfig)
{
	m_ptconfig = ptconfig;
}

int CDhtsSvrPerfTest::StartSvr( CSockAddrIPv4 & addr, bool seqttflag)
{
    if ( m_listener.Open( addr) < 0)
    {
        printf(" start serv failed\n");
        return -1;
    }

    RegisterSock(m_listener, READ_MASK);

	 m_seqttflag = seqttflag;

	 if (m_ptconfig.interval > 0 && m_seqttflag == 0 )
	 {
			m_thread = new CThreadLite( CThreadLite::ThreadProc( &CDhtsSvrPerfTest::ThreadProc, this ), "Dhtssvr_Monitor_Thread", 64, 0, 0 );
			m_thread->createThread();
	 }
	 
    return 0;
}

void CDhtsSvrPerfTest::StopSvr()
{
	if (m_thread)
	{
		m_thread->cancelThread();
		m_thread->destroyThread();
		delete m_thread, m_thread = NULL;
	}
	Close();
}

void CDhtsSvrPerfTest::Close()
{
	DestroySessionList();

	CNetHandler::Close();
}

ConnectSession *CDhtsSvrPerfTest::CreateSession(CDhtsStream *sock, bool seqttflag)
{
	ConnectSession *session = NULL;

	session = new ConnectSession;
	if (session != NULL)
	{
		session->stream = sock;
		session->seqttflag = seqttflag;
		if (!session->seqttflag)
		{
			FILE *fp = NULL;
			char filename[32];
			int len = snprintf(filename, sizeof(filename)-1, "%s%ld", FNAME_PRFIX, m_mnglist.size());
			filename[len] = 0;
			
			if (!(fp = fopen(filename, "a+")))
			{
				printf("fopen failed, %s\n", strerror(errno));
				//delete session, session=NULL;
				//return NULL;
			}
			
			session->fp = fp;
		}

		session->bufsize = RX_MAX_BUFSIZE;
		session->pcdata = new char[session->bufsize];
		if (session->pcdata==NULL)
		{
			printf("alloc rx buffer failed, no mem\n");
			if (session->fp)
				fclose(session->fp), session->fp = NULL;
			delete session, session=NULL;
			return NULL;
		}

		session->total_rxbytes = 0;
		session->total_txbytes = 0;
		session->rxbytes = 0;
		session->txbytes = 0;
		session->intval = 0;
		session->lastsamplets = CTime::getCurrentMilliSecond();
		return session;
	}

	return NULL;
}

void CDhtsSvrPerfTest::DestroySession(ConnectSession *session)
{
	//if (session->seqttflag)
	printf("conn<%d> recv total %d bytes data\n", session->stream->GetHandle(), session->total_rxbytes);
	
	if (session->pcdata)
	{
		delete [](session->pcdata);
		session->pcdata = NULL;
		session->bufsize = 0;
	}
	
	if (session->fp)
	{
		fclose(session->fp);
		session->fp = NULL;
	}

	if (session->stream)
	{
		session->stream->Close();
		delete session->stream;
		session->stream = NULL;
	}
}

int CDhtsSvrPerfTest::DestroySessionList()
{
	ConnectSession *session = NULL;
	ConnListIterator pit;

	m_mutex.enter();

	for (pit = m_mnglist.begin(); pit != m_mnglist.end();)
	{
		session = pit->second;
		m_mnglist.erase(pit++);
		if (session)
		{
			RemoveSock(*session->stream);
			DestroySession(session);
			delete session, session=NULL;
		}
	}

	m_mutex.leave();

	RemoveSock(m_listener);
	m_listener.Close();

	return 0;
}


int CDhtsSvrPerfTest::ProcessConnRequest()
{
	char ip_str[256];
	CDhtsStream *stream;
	CSockAddrIPv4 remote_addr;

	stream = m_listener.Accept( &remote_addr );
	if( stream != NULL )
	{
		ConnectSession *session;
		printf( "new client connect to server %s:%d, handle:%d\n", 
				remote_addr.GetIpStr(ip_str, sizeof(ip_str)), remote_addr.GetPort(), stream->GetHandle());
		
		if ( !(session = CreateSession(stream, m_seqttflag)))
		{
			printf("create connect session failed, handle:%d\n", stream->GetHandle());
			return -1;
		}
		
		m_mutex.enter();
		m_mnglist[stream->GetHandle()] = session;
		m_mutex.leave();
		
		RegisterSock( *stream, READ_MASK); 
	}
	return 0;

}

int CDhtsSvrPerfTest::ProcessData(int handle)
{
	int len = 0;
	ConnectSession *session = NULL;
	ConnListIterator pit;
	
	m_mutex.enter();
	pit = m_mnglist.find(handle);
	if (pit == m_mnglist.end())
	{
		m_mutex.leave();
		return 0;
	}
	if ((session = pit->second) != NULL)
	{
		len = session->stream->Recv(session->pcdata, session->bufsize);
		if (len < 0)
		{
			m_mnglist.erase(pit);
			m_mutex.leave();
			printf("remove connect session handle %d\n", session->stream->GetHandle());
			RemoveSock(*session->stream);
			DestroySession(session);
			delete session, session=NULL;
			return -1;
		}
		
		session->total_rxbytes += len;
		session->rxbytes += len;

		if (m_seqttflag)
		{
			printf("<%d>: %s\n", handle, session->pcdata);
		}
		else 
		{
			//printf("sockfd %d recv %d bytes data\n", handle, len);
			if (session->fp)
			{
				if (fwrite(session->pcdata, len, 1, session->fp) < 0)
				{
					printf("fwrite data to file failed, %s\n", strerror(errno));
				}
			}
		}

	}
	
	m_mutex.leave();

	return 0;
}

int CDhtsSvrPerfTest::handle_input( int handle )
{
	if( handle == m_listener.GetHandle() )
	{		
		return ProcessConnRequest();
	}
	else 
	{  
		return ProcessData(handle);
	}
	
	fprintf( stderr, "error, cannot run here\n" );
	return 0;			
}

int CDhtsSvrPerfTest::handle_input_timeout( int handle )
{
    
    printf("handle_input_timeout %d\n", handle);
	
	m_mutex.enter();
	std::map<int, ConnectSession*>::iterator pit = m_mnglist.find(handle);

	if (pit != m_mnglist.end())
	{
		ConnectSession *session = pit->second;

		if (session)
		{
			RemoveSock( *session->stream);
			m_mnglist.erase(pit);
			delete session, session=NULL;
		}
	}
	m_mutex.leave();

	fprintf( stderr, "error, cannot run here\n" );
	return 0;
}

int CDhtsSvrPerfTest::sample(ConnectSession *session, BandwithSample *bwsample, bool clear)
{
	uint64_t ts =  CTime::getCurrentMilliSecond();

	bwsample->rxbytes = session->rxbytes;
	bwsample->txbytes = session->txbytes;
	bwsample->intval_ms = ts - session->lastsamplets;

	if (bwsample->intval_ms > 0 && bwsample->rxbytes > 0)
		bwsample->rxbw = bwsample->rxbytes * 8.0 / (bwsample->intval_ms * 1000); 
	if (bwsample->intval_ms > 0 && bwsample->txbytes > 0)
		bwsample->txbw = bwsample->txbytes * 8.0 / (bwsample->intval_ms * 1000);

	if (clear)
	{
		session->rxbytes = 0;
		session->txbytes = 0;
		session->lastsamplets = ts;
	}
	return 0;
}

void CDhtsSvrPerfTest::ThreadProc(  CThreadLite& arg  )
{		
	int No;
	double intvl = 0.0;
	BandwithSample bws;
	ConnListIterator pit;
	std::map<int, AvgRateStatistic> avgrate; 
	std::map<int, AvgRateStatistic>::iterator pavgit;
	
	printf("[ ID] Interval\t\tTransfer(kBytes)\t\tBandwidth(Mbps)\n");

	while( arg.looping() )
	{
		CThread::sleep(m_ptconfig.interval * 1000);
	
		m_mutex.enter();
		for (pit = m_mnglist.begin(); pit != m_mnglist.end(); pit++)
		{
			No = 0;
			sample(pit->second, &bws, true);
			printf("[%-2d] %.2f-%2.f.00 sec \t\t%d KBytes \t\t%.2f Mbits/sec\n", 
				No, intvl, intvl+m_ptconfig.interval, bws.rxbytes /1000, bws.rxbw);

			if (bws.rxbw > 0)
			{
				pavgit = avgrate.find(No);
				if (pavgit == avgrate.end())
				{
					AvgRateStatistic rs;
					rs.total_bw = bws.rxbw;
					rs.count = 1;
					avgrate[No] = rs;
				}
				else
				{
					pavgit->second.count++;
					pavgit->second.total_bw += bws.rxbw;
				}
			}
			No++;
			intvl += 1;
		}
		m_mutex.leave();
	}

	printf("---------------------------------------------------------------------\n");
	printf("[ ID]\t\tInterval\t\tBandwidth\n");
	int i = 0;
	for (pavgit = avgrate.begin(); pavgit != avgrate.end(); pavgit++)
	{
		if (pavgit->second.count)
			printf("[ %-2d]\t\t%d\t\t\t%.2f\n", i, pavgit->second.count, pavgit->second.total_bw/pavgit->second.count);
		i++;
	}

	
}

