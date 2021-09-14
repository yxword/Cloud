//
//  "$Id: Timer.h 7672 2010-5-17 02:28:18Z he_linqiang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-5-17        he_linqiang Create
//
//
#ifndef __INCLUDED_TIMER_H__
#define __INCLUDED_TIMER_H__

#include "NetFramework/NetHandler.h"
using namespace Dahua::NetFramework;
class CTimer : public CNetHandler
{
public:
	CTimer();
	virtual ~CTimer();
public:
	int StartTimer();
	int64_t handle_timeout( long id );
	int handle_close( CNetHandler* myself );
private:
	long	m_timer_id;
};

int StartTimerTest();

#endif //__INCLUDED_TIMER_H__
