#include "DhtsCli.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "Infra/Thread.h"
CDhtsCli::CDhtsCli()
{
}

CDhtsCli::~CDhtsCli()
{
}

int CDhtsCli::StartSession( CSockAddr & addr, bool initial )
{
    CSockAddrIPv4 local("10.6.5.215", 10000);

	if (initial)
	{
		//这个判断没有必要，Connect方法里会判断，写在这里是为了帮助更好地理解地址对象。
		if( addr.GetType() != CSockAddr::SOCKADDR_TYPE_IPV4 ){
			fprintf( stderr, "地址类型错误！\n");
			return -1;
		}
		//Connect返回-1则直接失败，但返回大于等于0不表示成功。
		if( m_stream.Connect( addr, (CSockAddr *)&local) < 0 ){
			printf( "连接直接失败！\n" );
			return -1;
		}
		//注册写事件，连接成功或失败，写回调都会被调用，可在那里对连接状态进行状态。
		//同时注册一个5秒超时，5秒之后如果还处在未连接中，则可在回调函数中对它进行处理。
		RegisterSock( m_stream, WRITE_MASK|READ_MASK);
	}
	else 
		RegisterSock( m_stream, WRITE_MASK|READ_MASK, 5000000);
	return 0;
}

int CDhtsCli::handle_input(int handle)
{
    if (handle == m_stream.GetHandle())
    {
        char msgbuf[128];
        snprintf(msgbuf, sizeof(msgbuf)-1, "%s", "<tim> ");

        int ret = m_stream.Recvmsg(msgbuf+7, sizeof(msgbuf)-8);
        if (ret < 0)
        {
            printf("recv failed\n");
            return -1;
        }
        else if (ret == 0)
            return 0;
        else 
        {
            printf("%s\n", msgbuf);
            return 0;
        }
    }
}

int CDhtsCli::handle_input_timeout(int handle)
{
    printf("%d handle_input_timeout\n", handle);

    if (handle == m_stream.GetHandle())
    {   
        printf( "连接已超时!\n" );
        RemoveSock( m_stream );
        m_stream.Close();
    }
    return -1;
}

int CDhtsCli::handle_output( int handle )
{

	//printf("handle: %d\n", handle);

	//如果m_stream在发送数据的过程中因为发不出去注册了写事件，则这里要判断
	//当前的状态：如果还未连接，则判断连接状态，如果处在发数据过程中，则继续发送未发完的数据。
	if( handle == m_stream.GetHandle() ){		//属于m_stream的事件，
		int ret = m_stream.GetConnectStatus();
		if( ret == CDhtsStream::STATUS_NOTCONNECTED ){
			printf( "正在连接中...!\n" );
			return 0;						//还在连接，返回0，维持5秒超时的设置。
		}else if( ret == CDhtsStream::STATUS_ERROR ){
			printf( "连接失败！ \n" );
			RemoveSock( m_stream );			//注意，不用的socket一定要删除注册。
			return -1;						//注册已删除，返回值无意义。
		}else{
			//printf( "连接成功!\n" );
			UnregisterSock( m_stream, WRITE_MASK );	//连接成功，则写事件可注销。
            //RemoveSock(m_stream);

			do_send();			//发送数据。如果没有全发送出去，这里应该注册写事件。
			return 0;			//返回-1，删除5秒超时（其实目前无事件被注册，5秒超时不会发生作用）。
		}
	}
	return 0;
}

int CDhtsCli::handle_close(CNetHandler* myself )
{
	delete this;
}

int CDhtsCli::do_send()
{
	char send_buf[256];
	time_t ticks = time(NULL);
    static int cnt=0;
	int len = snprintf(send_buf, sizeof(send_buf)-1, "%s", ctime(&ticks));
	send_buf[len] = 0;

	int ret = m_stream.Sendmsg( send_buf, len+1);
	if( ret < 0 )
		fprintf( stderr, "数据发送错误!\n" );

    printf("[%d] %s\n", cnt++, send_buf);
	return 0;
}

int CDhtsCli::handle_output_timeout( int handle )
{
	printf( "超时！\n" );
	if( handle == m_stream.GetHandle() ){
		printf( "连接已超时!\n" );
		RemoveSock( m_stream );
	}
	return -1;			//超时机制一律不再保留。
}

int CDhtsCli::handle_exception( int handle )
{
	if( handle == m_stream.GetHandle() ){
		printf( "连接发生异常!\n" );
		RemoveSock( m_stream );
	}
	return -1;
}

#if 0
#endif 

int StartDhtsCli()
{
	CDhtsCli *pdhtscli = new CDhtsCli;
	
	CSockAddrIPv4 addr( "10.6.5.215", 9000);
	if( pdhtscli->StartSession( addr, 1) < 0 )
		return -1;
		
	// 如果输入q 字符退出
	char c = 0;
	while(c = getchar())
	{
		if ('q' == c)
		{
			printf("exit \n");
			break;
		}

		Dahua::Infra::CThread::sleep(1 * 1000);
		
		if( pdhtscli->StartSession( addr, 0) < 0 )
			break;
	}
	
	if (pdhtscli)
		pdhtscli->Close();
	
	Dahua::Infra::CThread::sleep(2 * 1000);

	return 0;
}