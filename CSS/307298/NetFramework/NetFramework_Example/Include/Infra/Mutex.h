//
//  "$Id: Mutex.h 55347 2012-03-22 11:26:45Z qin_fenglin $"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INFRA3_MUTEX_H__
#define __INFRA3_MUTEX_H__

#include "Defs.h"


namespace Dahua{
namespace Infra{

struct MutexInternal;

/// \class CMutex
class INFRA_API CMutex
{
	CMutex(CMutex const&);
	CMutex& operator=(CMutex const&);

public:
	/// 构造函数，会创建系统互斥量
	CMutex();

	/// 析构函数，会销毁系统互斥量
	~CMutex();

	/// 进入临界区。
	/// \return 操作是否成功
	bool enter();

	/// 离开临界区。
	/// \return 操作是否成功
	bool leave();

private:
	MutexInternal *m_internal;
};

} // namespace Infra
} // namespace Dahua

#endif //__INFRA_MUTEX_H__

//
// End of "$Id: Mutex.h 55347 2012-03-22 11:26:45Z qin_fenglin $"
//
