//
//  "$Id: Semaphore.h 55347 2012-03-22 11:26:45Z qin_fenglin $"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INFRA3_SEMAPHORE_H__
#define __INFRA3_SEMAPHORE_H__

#include "Defs.h"

#if defined(__linux__)
#include <semaphore.h>
#endif

namespace Dahua{
namespace Infra{

struct SemaphoreInternal;

/// \class CSemaphore
/// \brief 多平台信号量类
class INFRA_API CSemaphore
{
	CSemaphore(CSemaphore const&);
	CSemaphore& operator=(CSemaphore const&);

public:
	/// 构造函数，会创建系统信号量
	/// \param initialCount 信号量初始计数
	explicit CSemaphore(int initialCount = 0);

	/// 析构函数，会销毁系统互斥量
	~CSemaphore();

	/// 减少信号量计数，如果已经减少到0，会阻塞调用的线程
	/// \return 当前信号量计数
	int pend();

	/// 增加信号量计数，如果是从0累加，会唤醒其等待队列的第一个线程
	/// \return 当前信号量计数
	int post();

private:
	SemaphoreInternal* m_internal;
};

} // namespace Infra
} // namespace Dahua

#endif //__INFRA_SEMAPHORE_H__
//
// End of "$Id: Semaphore.h 55347 2012-03-22 11:26:45Z qin_fenglin $"
//
