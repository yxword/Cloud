#include "NTimerTest.h"

#include <stdio.h>

NetFramework::CNTimer	*CAliveTimer::m_static_alivetimer = NULL;
Infra::CMutex				CAliveTimer::m_static_alivemutex;

CAliveTimer* CAliveTimer::create(long ownerid)
{
	Infra::CGuard guard(m_static_alivemutex);
	if(!m_static_alivetimer)
		m_static_alivetimer = NetFramework::CNTimer::Create(1 * 1000000);
	return new CAliveTimer(ownerid);
}

CAliveTimer::CAliveTimer(long ownerid):m_ref_count(0)
{
	if (m_static_alivetimer)
		m_static_alivetimer->PushTimerEvent(this);
}

CAliveTimer::~CAliveTimer()
{

}

int CAliveTimer::startAliveTimer()
{
	NetFramework::CNTimerEvent::Start();	///> 开启定时器
	return 0;
}

int CAliveTimer::stopAliveTimer()
{
	NetFramework::CNTimerEvent::Stop();		///>  停止定时器
	
	return 0;
}

void CAliveTimer::destroy()
{
	Close();		///> 网络框架内部自动回收资源
}
	
int CAliveTimer::handle_timer_event()
{
	printf("CAliveTimer::handle_timer_event\n");	
	m_ref_count++;
	
	return 0;
}

int CAliveTimer::handle_close( NetFramework::CNetHandler* myself )
{
	delete this;
	return -1;
}

