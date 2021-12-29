#ifndef __DAHUA_VIDEOCLOUD_DBHA_WAITER_H__
#define __DAHUA_VIDEOCLOUD_DBHA_WAITER_H__

#include "Infra/IntTypes.h"
#include "Infra/Mutex.h"
#include "Infra/Semaphore.h"

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

class CWaiter
{
public:
	CWaiter( void );
	~CWaiter( void );

	//同步阻塞.
	int32_t pend( int32_t timeout );

	//停止阻塞pend结束.
	int32_t post();

private:
	Infra::CMutex m_mutex;
	Infra::CSemaphore* m_sem;
};

} // DBHA
} // VideoCloud
} // Dahua

#endif // __DAHUA_VIDEOCLOUD_DBHA_WAITER_H__
