#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"
#include "NetFramework/SockDgram.h"
#include "NetFramework/SslDgram.h"
#include "Infra/Time.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "dhtssvr_perftest.h"
#include "dhtscli_perftest.h"
#include "Infra/Time.h"

using namespace Dahua::NetFramework;

//#define MB_PER_BITS	(1024 * 1024)

enum 
{
	MODE_SVR=1,
	MODE_CLI=2
};

class CConfigParser
{
public:
	CConfigParser()
	{
		memset(&m_ptconfig, 0, sizeof(m_ptconfig));
	}
	~CConfigParser(){}

	void getConfigInfo(PerfTestConfig &ptconfig)
	{
		memcpy(&ptconfig, &m_ptconfig, sizeof(PerfTestConfig));
	}

	PerfTestConfig &getConfigInfo()
	{
		return m_ptconfig;
	}

	int getCfgChanNum()
	{
		return m_ptconfig.nchans;
	}

	int getCfgBandwidth()
	{
		return m_ptconfig.bandwidth;
	}

	int getCfgInterval()
	{
		return m_ptconfig.interval;
	}

	int getCfgDuration()
	{
		return m_ptconfig.duration;
	}
	
	int getCfgMode()
	{
		return m_ptconfig.mode;
	}

	int getCfgPort()
	{
		return m_ptconfig.port;
	}

	int getCfgFps()
	{
		return m_ptconfig.fps;
	}

	int getCfgDrvCnt()
	{
		return m_ptconfig.drvcnt;
	}

	int getSeqttFlag()
	{
		return m_ptconfig.seqtimetest;
	}
	
	
	int parseUserOption(int argc, char **argv)
	{
		char opt;
		
		while ( (opt = getopt(argc, argv, "b:dP:t:i:sc:p:f:n:T")) != -1) 
		{   
		    switch (opt)
		    {   
		        case 'b':
		            m_ptconfig.bandwidth = atoi(optarg);    
		            break;
		        case 'd':
		            m_ptconfig.debug = 1;
		            break;
		        case 'P':
		            m_ptconfig.nchans = atoi(optarg);
		            break;
		        case 'i':
		            m_ptconfig.interval= atoi(optarg);
		            break;
		        case 't':
		            m_ptconfig.duration = atoi(optarg);
		            break;
				case 's':
					m_ptconfig.mode = MODE_SVR;
					break;
				case 'c':
					m_ptconfig.mode = MODE_CLI;
					inet_pton(AF_INET, optarg, &m_ptconfig.addr.sin_addr);
					break;
				case 'p':
					m_ptconfig.port = atoi(optarg);
					break;
				case 'f':
					m_ptconfig.fps = atoi(optarg);
					break;
				case 'n':
					m_ptconfig.drvcnt = atoi(optarg);
					break;
				case 'T':
					m_ptconfig.seqtimetest = 1;
					break;
		        case '?':
		        default :
		            printf("invalid opt\n");
		            return -1;
		    }   
		} 

		if (m_ptconfig.mode != MODE_CLI && m_ptconfig.mode != MODE_SVR)
		{
			printf("invalid mode param: %d\n", m_ptconfig.mode);
			return -1;
		}

		if (m_ptconfig.mode == MODE_CLI)
		{
			if (m_ptconfig.nchans <= 0)
			m_ptconfig.nchans = 1;

			if (m_ptconfig.bandwidth <= 0)
				m_ptconfig.bandwidth = 2;

			m_ptconfig.bandwidth *= 1024*1024;

			if (m_ptconfig.duration <= 0)
				m_ptconfig.duration = 10;	// 10s 

			if (m_ptconfig.fps <= 0)
				m_ptconfig.fps = 25;
				
			m_ptconfig.addr.sin_family = AF_INET;
			m_ptconfig.addr.sin_port = htons(m_ptconfig.port);
		}

		return 0;
	}

	static void usage(char *prgname)
	{
		printf("Usage: %s <-s>|<-c host> <-p port> [-b bw] [-P nchans] [-d] [-i interval] [-t duration]\n", prgname);
	}

	void printConfig()
	{
		printf("user option info:\n");
		printf("\tnchans:%d bw:%d intvl:%d dgb:%d duration:%d port:%d, ipaddr:%s, mode:%d, fps:%d, drvcnt:%d, seqtt:%d\n", 
				m_ptconfig.nchans, m_ptconfig.bandwidth, m_ptconfig.interval, m_ptconfig.debug,
				m_ptconfig.duration, m_ptconfig.port, inet_ntoa(m_ptconfig.addr.sin_addr), m_ptconfig.mode, 
				m_ptconfig.fps, m_ptconfig.drvcnt, m_ptconfig.seqtimetest
				);
	}
private:
	PerfTestConfig m_ptconfig;
};


int main( int argc, char **argv )
{
	CConfigParser cfgparser;

	if (argc < 3)
	{
		CConfigParser::usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	if (cfgparser.parseUserOption(argc, argv) < 0)
		exit(EXIT_FAILURE);
	
	cfgparser.printConfig();

	if( CNetThread::CreateThreadPool( 4, true ) < 0 ){
		fprintf( stderr, "创建线程池失败!\n" );
		CNetThread::DestroyThreadPool();
		return -1;
	}

	if (cfgparser.getCfgMode() == MODE_SVR)
	{
		CDhtsSvrPerfTest dhtssvr;		
		CSockAddrIPv4 addr( INADDR_ANY, (unsigned short)cfgparser.getCfgPort()); 	
		
		dhtssvr.SetPtconfig(cfgparser.getConfigInfo());
		if( dhtssvr.StartSvr( addr, cfgparser.getSeqttFlag() ) < 0 )
			return -1;
		
	    fprintf(stdout, "start dhts server on %d port\n", cfgparser.getCfgPort());

		char ch;
		while(1)
		{
			//Dahua::Infra::CThread::sleep( 1 * 1000 );
			
			ch = getchar(); getchar();
			if ( ch == 'q' || ch == 'Q')
				break;
		}
		dhtssvr.StopSvr();
		CThread::sleep(1000);
	}
	else if (cfgparser.getCfgMode() == MODE_CLI)
	{
		int txintvl;
		uint64_t expire_ms;
		CDhtsCliPerfTest dhtscli;

		int drvcnt = cfgparser.getCfgDrvCnt();
		dhtscli.SetPtconfig(cfgparser.getConfigInfo());

		if (cfgparser.getSeqttFlag())
			txintvl = 1 * 1000;    // 1s interval 
		else 
			txintvl = 1 * 1000/cfgparser.getCfgFps();  // 40 ms 
		
		expire_ms = Dahua::Infra::CTime::getCurrentMilliSecond() + cfgparser.getCfgDuration() * 1000;
	
		if( dhtscli.StartSession() < 0 )
			return -1;
		
		while (true)
		{
			//printf("getTimeofms:%lld\n", Dahua::Infra::CTime::getCurrentMilliSecond());
			
			Dahua::Infra::CThread::sleep(txintvl);

			// if numdrv is set, check it first 
			if (drvcnt>0)
			{
				if (--drvcnt == 0)
					break;
			}
			else 
			{
				if (Dahua::Infra::CTime::getCurrentMilliSecond() > expire_ms)
					break;
			}
		
			dhtscli.RestartSession();
		}

		dhtscli.Close();
	}

		// delay 
	sleep(10);
	printf("main over\n");

	CNetThread::DestroyThreadPool();
	return 0;
}
