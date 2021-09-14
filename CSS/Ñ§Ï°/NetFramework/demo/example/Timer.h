//
//  "$Id: Timer.h 31672 2016-03-18 09:03:00 wang_zhihao $"
//
//  Copyright (c)1992-2016, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2016-3-18		wang_zhihao	Create
//

#ifndef __INCLUDED_TIMER_H__
#define __INCLUDED_TIMER_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/NetThread.h"
#include "Infra/Thread.h"

#include <iostream>

using namespace Dahua::NetFramework;

class CTimer : public CNetHandler
{
public:
     CTimer( int64_t interval )
     	: m_timer_id( 0 )
        , m_timer_interval( interval )
     {
     	
     }
     virtual ~CTimer()
     {
     }
     
     void startTimer()
     {
     	m_timer_id = SetTimer( m_timer_interval );
     }

     void destroyTimer()
     {
        DestroyTimer(m_timer_id);
     }

     int64_t handle_timeout( long id )
     {
     	std::cout << "Timer " << id
     			  << " timeout, thread id " << Dahua::Infra::CThread::getCurrentThreadID()
     	          << std::endl;
	
        //返回0继续下一个超时
        return 0;
     }

     int handle_close( CNetHandler* myself )
     {
         std::cout << "timer close, thread id " << Dahua::Infra::CThread::getCurrentThreadID() << std::endl;

         return 0;
     }

private:
     long m_timer_id;
     int64_t m_timer_interval;
};

#endif //__INCLUDED_TIMER_H__
