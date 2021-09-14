#include "Timer.h"

#include <stdio.h>
#include "Infra/Thread.h"
CTimer::CTimer()
	:m_timer_id(0)
{
}
	
CTimer::~CTimer()
{
	printf( "by handle_close delete，or other\n" );
}

int CTimer::StartTimer()
{
	m_timer_id = SetTimer( 1000000 );	//1秒钟后触发一个超时事件。
	return 0;
}
	
int64_t CTimer::handle_timeout( long id )
{
	if( id == m_timer_id ){		//确认超时定时器的有效性。
		Close();								//结束对象。
	}else
		fprintf( stderr, "Invalid！\n" ); //如果执行到这句，是属于框架的bug.
		
	return -1;	//销毁定时器。此时因为已经Close，大于等于0的返回也无效，定时器已经销毁。
}

int CTimer::handle_close(CNetHandler *myself)
{
	delete myself;
}

#if 0
#endif 

int StartTimerTest()
{
	CTimer *timer = new CTimer;
	if( timer->StartTimer() < 0 )
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
	}
	
	if (timer)
		timer->Close();
		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}