#include "HttpSvr.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>


#include "Infra/Time.h"
#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"

#define WEBSVR_ROOT_PATH "html/"

enum LogLevel
{
	LOG_LV_DETAIL,
	LOG_LV_DEBUG,
	LOG_LV_INFO,
	LOG_LV_WARNING,
	LOG_LV_ERROR
};

static int logLevel = LOG_LV_INFO;


CHttpSvr::CHttpSvr()
{
}

CHttpSvr::~CHttpSvr()
{
}

int CHttpSvr::StartSvr( CSockAddrIPv4 & addr )
{	
	if( m_acceptor.Open( addr ) < 0 ){
		fprintf( stderr, "open acceptor failed\n" );
		return -1;
	}	
	RegisterSock( m_acceptor, READ_MASK );
	return 0;
}

/*
	GET /index.html HTTP/1.1
	User-Agent: xxx
	Host: localhost:9999
	Accept: xxx
*/ 
int CHttpSvr::httpRequestParser(char *reqBufmsg, HttpReqeust &request)
{
	char *line_start = NULL;
	char *line_end = NULL;
	char *url_start = NULL;
	char *url_end = NULL;
	char *ver_start  = NULL;
	
	url_start = line_start = reqBufmsg;
	if (!url_start)
	{
		printf("invalid req msg, req msg null\n");
		return -1;
	}

	url_end = strstr(url_start, "\r\n\r\n");
	if (!url_end) 
	{	
		printf("invalid req msg, need header termination\n");
		return -1;
	}

	line_end = strstr(url_start, "\r\n");
	ver_start = strstr(url_start, " HTTP");
	if (!line_start || !ver_start)
	{
		printf("invalid req msg, need line term or version\n");
		return -1;
	}

	if (0 == strncmp(line_start, "GET ", 4))
	{
		request.method = REQ_METHOD_GET;
		line_start += 4;
	}
	else if ( 0 == strncmp(line_start, "POST ", 5))
	{
		request.method = REQ_METHOD_POST;
		line_start += 5;
	}
	else 
	{
		printf("Not support HTTP request, %s\n", line_start);
		return -1;
	}

	// trim url path
	char *ptr = line_start;	
	char *path_start = ptr;
	do 
	{
		if (*ptr++ == '/')
			line_start = ptr;
	} while (ptr < ver_start);

	strncpy(request.urlpath, path_start, line_start - path_start);
	strncpy(request.filename, line_start, ver_start - line_start);
	
	return 0;
}


static char errstr[] = 
"HTTP/1.1 404 Not Found\r\n"
"Server: ntfSvr/3.x\r\n";
 
int CHttpSvr::doHttpReqProc(CSockStream *stream, char *reqBufmsg)
{
	HttpReqeust request;

	if (httpRequestParser(reqBufmsg, request) < 0 )
	{
		printf("HttpRequestParser failed, handle %d\n", stream->GetHandle());
		return -1;
	}

	if (logLevel <= LOG_LV_DEBUG)
	{
		printf("Method: [%s]\n", request.method == REQ_METHOD_POST?"POST":"GET");
		printf("UrlPath: [%s]\n", request.urlpath);
		printf("Filename: [%s]\n", request.filename);
	}

	char filePath[64] = {0};
	snprintf(filePath, sizeof(filePath)-1, "%s%s", WEBSVR_ROOT_PATH, request.filename);

	FILE *fp = fopen(filePath, "r");
	if (fp==NULL)
	{		
		printf("fopen %s failed, %s\n", filePath, strerror(errno));		
		stream->Send(errstr, strlen(errstr) + 1);
	}
	else 
	{
		int len = 0;
		char context[1024] = {0};
		
		len = fread(context, 1, sizeof(context), fp);
		fclose(fp);

		if (logLevel <= LOG_LV_DEBUG)
			printf("Read from file %s %d bytes\n", filePath, len);

			    /* send data */
	    char *p_bufs = (char *)malloc(len + 1024);
	    if (NULL == p_bufs)
	    {
	        printf("malloc error!\n");
	        stream->Send(errstr, strlen(errstr) + 1);
	    }
		else 
		{
			    
		    int tlen = sprintf(p_bufs,	"HTTP/1.1 200 OK\r\n"
		                        "Server: ntfSvr/3.x\r\n"
		                        "Content-Type: text/html\r\n"
		                        "Content-Length: %d\r\n"
		                        "Connection: keep-alive\r\n\r\n",
		                        len);

		    memcpy(p_bufs+tlen, context, len);
		    tlen += len;
		    int wr = stream->Send(p_bufs, tlen);
			if (logLevel <= LOG_LV_DETAIL)
				printf("send %d bytes to peer\n", wr);
		    free(p_bufs);	
		}
	}
	
	return 0;
}

int CHttpSvr::procConnRequest(int handle)
{
	CSockStream *pstream;
	ConnSockTableIt cstIt;

	m_mutex.enter();
	cstIt = m_tables.find(handle);
	if ( cstIt != m_tables.end() )
	{
		pstream = cstIt->second;
		m_mutex.leave();
		
		char recv_buf[1024];
		memset( recv_buf, 0, sizeof(recv_buf) );
		int ret = pstream->Recv( recv_buf, sizeof(recv_buf) );
		if (ret < 0)
		{
			if( errno == ECONNRESET )
			{
				if (logLevel <= LOG_LV_INFO)
					fprintf( stderr, "peer closed, handle: %d\n",  pstream->GetHandle());
			}
			else
			{
				fprintf( stderr, "recv data failed, handle: %d\n", pstream->GetHandle());	
			}

			m_mutex.enter();
			m_tables.erase(cstIt);
			m_mutex.leave();
			RemoveSock( *pstream );	
			delete pstream, pstream=NULL;
		}
		else
		{
			if (logLevel <= LOG_LV_DEBUG)
			{	
				printf("---------------------------\n");
				printf("%s", recv_buf);
			}

			doHttpReqProc(pstream, recv_buf);
		}
	}
	else 
	{
		printf("not connected handle, %d\n", handle);
		m_mutex.leave();
	}
	
	return 0;
}

int CHttpSvr::handle_input( int handle )
{
	if( handle == m_acceptor.GetHandle() )
	{		
		char ip_str[256] = {0};
		CSockStream *stream;
		CSockAddrIPv4 remote_addr;

		stream = m_acceptor.Accept( &remote_addr );
		if( stream != NULL )
		{			
			if (logLevel <= LOG_LV_INFO)
				printf("accept a new client： %s:%d\n", 
					remote_addr.GetIpStr(ip_str, sizeof(ip_str)), remote_addr.GetPort() );

			m_mutex.enter();
			m_tables[stream->GetHandle()] = stream;
			m_mutex.leave();
			
			RegisterSock( *stream, READ_MASK );	
		}
	}
	else
	{  
		procConnRequest(handle);
	}
	return 0;			
}

int CHttpSvr::handle_input_timeout( int handle )
{
	fprintf( stderr, "错误，不可能运行到这儿!\n" );
	return 0;
}

int CHttpSvr::handle_close(CNetHandler *myself)
{
	delete myself;
	return 0;
}

#if 0
#endif 

static void usage(char *program)
{
	printf("Usage: %s [loglv]\n", program);
	printf("---------------------------------\n");
	printf("\tlovlv\t 1\t detail\n");
	printf("\t     \t 2\t debug\n");
	printf("\t     \t 3\t info\n");
	printf("\t     \t 4\t warning\n");
	printf("\t     \t 5\t error\n");
}

/*
	ab -k -c 1 -n 1 http://10.6.5.215:9999/index.html
*/
int StartHttpSvr(int argc, char **argv)
{
	if (argc >= 2)
	{
		if ( 0 == strncmp(argv[1], "-h", 2))
		{
			usage(argv[0]);
			return 0;
		}
		else 
		{
			int lvno = atoi(argv[1]);
			switch (lvno) {
				case 1:
					logLevel = LOG_LV_DETAIL;
					break;
				case 2:
					logLevel = LOG_LV_DEBUG;
					break;
				case 3:
					logLevel = LOG_LV_INFO;
					break;
				case 4:
					logLevel = LOG_LV_WARNING;
					break;
				case 5:
					logLevel = LOG_LV_ERROR;
					break;
				default:
					printf("invalid log level, %d\n", lvno);
					usage(argv[0]);
					return -1;
			}
		}
	}
	CHttpSvr *ptcpsvr = new CHttpSvr;		
	CSockAddrIPv4 addr( INADDR_ANY, 9999 ); 	
	if( ptcpsvr->StartSvr( addr) < 0 )
	{	
		printf("start svr failed\n");
		return -1;
	}
	
	// 如果输入q 字符退出
	char c = 0;
	while((c = getchar()))
	{
		if ('q' == c)
		{
			printf("exit \n");
			break;
		}

		Dahua::Infra::CThread::sleep(1 * 1000);
	}
	
	if (ptcpsvr)
		ptcpsvr->Close();
		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}
