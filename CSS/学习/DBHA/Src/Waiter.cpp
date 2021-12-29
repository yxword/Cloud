#include "Waiter.h"
#include "HALog.h"

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

DBHALOG_CLASSNAME( CWaiter );

CWaiter::CWaiter( void )
{
	m_sem = NULL;
}

CWaiter::~CWaiter( void )
{
	if( m_sem ){
		m_mutex.enter();
		delete m_sem;
		m_sem = NULL;
		m_mutex.leave();
	}
}

int32_t CWaiter::pend( int32_t timeout )
{
	m_mutex.enter();
	if( m_sem ){
		delete m_sem;
		m_sem = NULL;
	}
	m_sem = new Infra::CSemaphore();
	if( m_sem==NULL ){
		DBHALOG_ERROR( "new CSemaphore failed.\n" );
		m_mutex.leave();
		return -1;
	}
	m_mutex.leave();

	int32_t ret = m_sem->pend( timeout ); //windows可能会返回258,此项目目前不支持windows.
	
	m_mutex.enter();
	delete m_sem;
	m_sem = NULL;
	m_mutex.leave();

	return ret;
}

int32_t CWaiter::post()
{
	int32_t ret = 0;
	m_mutex.enter();
	if( m_sem ){
		ret = m_sem->post();
	}
	m_mutex.leave();

	return ret;
}

} // Cluster
} // VideoCloud
} // Dahua

