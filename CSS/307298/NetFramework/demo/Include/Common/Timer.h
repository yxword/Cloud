//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_TIMER_H__
#define __INCLUDE_DAHUA_EFS_TIMER_H__

#include "Infra/Function.h"

namespace Dahua {
namespace EFS {

class CTimer
{
public:
	CTimer();
	~CTimer();
	static CTimer* instance();	

	typedef Infra::TFunction1<void,int64_t> TimeoutProc;	

	//注册回调，返回唯一id
	int64_t attach( TimeoutProc& proc );

	//通过id注销回调
	bool detach( int64_t id, bool wait = false );

	//关闭
	void close(); 
private:
	struct Internal;
	struct Internal*		m_internal;
};

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_TIMER_H__
