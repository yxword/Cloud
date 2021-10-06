#include "Timer.h"

#include <stdio.h>
#include "Infra/Thread.h"
CTimer::CTimer()
	:m_timer_id(0)
{
}
	
CTimer::~CTimer()
{
	printf( "by handle_close delete��or other\n" );
}

int CTimer::StartTimer()
{
	m_timer_id = SetTimer( 1000000 );	//1���Ӻ󴥷�һ����ʱ�¼���
	return 0;
}
	
int64_t CTimer::handle_timeout( long id )
{
	if( id == m_timer_id ){		//ȷ�ϳ�ʱ��ʱ������Ч�ԡ�
		Close();								//��������
	}else
		fprintf( stderr, "Invalid��\n" ); //���ִ�е���䣬�����ڿ�ܵ�bug.
		
	return -1;	//���ٶ�ʱ������ʱ��Ϊ�Ѿ�Close�����ڵ���0�ķ���Ҳ��Ч����ʱ���Ѿ����١�
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
	
	// �������q �ַ��˳�
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