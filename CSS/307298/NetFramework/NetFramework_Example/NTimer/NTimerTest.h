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
#ifndef __INCLUDED_NTIMER_TEST_H__
#define __INCLUDED_NTIMER_TEST_H__

#include "NetFramework/NetHandler.h"
using namespace Dahua::NetFramework;


///\brief 会话级别的定时器(10秒级)，供会话保活使用
class CAliveTimer : public NetFramework::CNTimerEvent
{
public:
	static CAliveTimer* create();
	
	///\brief 启动定时器
	int startAliveTimer();
	
	///\brief 停止定时器
	int stopAliveTimer();
	
	///\brief 释放定时器资源
	void destroy();
	
private:
	///\brief 构造函数
	CAliveTimer();
	
	///\brief 析构函数
	~CAliveTimer();

	///\brief 基类接口，供网络框架NTimer使用
	int handle_timer_event();	

	///\brief 网络框架基类接口
	int handle_close( NetFramework::CNetHandler* myself );
private:
	int					m_ref_count;						///< 用于定时器超时计数
	static NetFramework::CNTimer	*m_static_alivetimer;	///< 全局定时器
	static Infra::CMutex			m_static_alivemutex;							///< 定时器所属者id,  定时器可以向该id 发送消息
};


#endif //__INCLUDED_NTIMER_TEST_H__
