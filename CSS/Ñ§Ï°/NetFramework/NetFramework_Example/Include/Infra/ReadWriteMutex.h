//
//  "$Id$"
//
//  Copyright (c)1992-2011, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INFRA3_RW_MUTEX_H__
#define __INFRA3_RW_MUTEX_H__

#include "Defs.h"


namespace Dahua{
namespace Infra{

struct RWMutexInternal;

/// \class CMutex
/// \brief 多平台互斥量类
class INFRA_API CReadWriteMutex
{
	CReadWriteMutex(CReadWriteMutex const&);
	CReadWriteMutex& operator=(CReadWriteMutex const&);

public:
	/// 构造函数，会创建系统互斥量
	CReadWriteMutex();

	/// 析构函数，会销毁系统互斥量
	~CReadWriteMutex();

	/// 进入临界区。
	/// \return 操作是否成功
	bool enterReading();

	/// 进入临界区。
	/// \return 操作是否成功
	bool enterWriting();

	/// 离开临界区。
	/// \return 操作是否成功
	bool leave();

private:
	RWMutexInternal* m_internal;
};

} // namespace Infra
} // namespace Dahua

#endif //__INFRA_RW_MUTEX_H__

//
// End of "$Id: Mutex.h 16503 2010-11-26 02:23:15Z wang_haifeng $"
//
