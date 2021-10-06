//
//  "$Id: NTimer.h 7672 2012-11-01 02:28:18Z he_linqiang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2012-11-01		ye_zhidong	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_NTIMER_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_NTIMER_H__

#include "NDefs.h"
#include "Infra/IntTypes.h"
#include "NetHandler.h"

namespace Dahua{
namespace NetFramework{

class CNTimerEvent;
//定时事件管理类，定时时间到后集中触发所有的定时事件
class NETFRAMEWORK_API CNTimer : public CNetHandler
{
public:
	//创建CNTimer
	//参数：usec：定时触发时间，单位微秒
	//返回值：CNTimer对象指针，销毁时需调用Close
	static CNTimer* Create( int64_t usec );
	//关闭
	void Close();
public:
	//加入定时事件
	//参数：event：定时事件，定时事件到后触发该事件
	//返回值：0成功
	int	PushTimerEvent( CNTimerEvent* event );
private:
	struct Internal;
	struct Internal*	m_internal;
};

//定时事件类
class NETFRAMEWORK_API CNTimerEvent : public CNetHandler
{
public:
	CNTimerEvent();
	~CNTimerEvent();
public:
	//开始接受定时触发
	void Start();
	//停止接受定时触发
	void Stop();
	//重载Close
	void Close();
	//需要用户自己实现的定时事件处理函数,定时触发后会调用该函数
	virtual int handle_timer_event(){ return -1; }
private:
	struct Internal;
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_NTIMER_H__
