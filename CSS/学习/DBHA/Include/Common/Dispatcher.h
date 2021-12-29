#ifndef __INCLUDE_DAHUA_EFS_DISPATCHER__H__
#define __INCLUDE_DAHUA_EFS_DISPATCHER__H__

#include "Infra/Function.h"
#include "Infra/Mutex.h"
#include "Infra/Semaphore.h"
#include "Memory/SharedPtr.h"

namespace Dahua{
namespace EFS{

class CAsyncConnection;
/**
 * @brief 处理连接事件
 * @details 外部通过init()接口注册事件处理函数，包括连接成功、连接异常或失败、连接关闭。
 *			当调用destroy()后，调用handle*()这些函数时将会返回false。
 *			如果调用destroy()时，handle*()正在执行，则会等待handle*()执行结束。
 *			如果handle*()执行的回调函数中调用了destroy()，则依赖递归锁保证不会发生死锁。
 * @note 推荐以智能指针DispatcherPtr的方式使用
 */
class CDispatcher
{
public:
	typedef Infra::TFunction1<void, CAsyncConnection*> Handler;
public:
	CDispatcher();
	
	~CDispatcher();

	/**
	 * @brief 初始化
	 * @param connectedHandler 处理连接成功事件的handler
	 * @param errorHandler 处理连接失败或异常事件的handler
	 * @param closeHandler 处理连接关闭事件的handler
	 * @note 用户保证单线程调用
	 */
	void init( Handler connectedHandler, Handler errorHandler, Handler closeHandler );
	
	/**
	 * @brief 销毁Dispacther
	 */
	void destroy();
	
	/**
	 * @brief 处理connection已经连接事件
	 * @param conn 连接对象
	 * @retval true 可处理
	 * @retval false Dispatcher已经被销毁
	 */
	bool handleConnConnected( CAsyncConnection* conn );

	/**
	 * @brief 处理connection连接错误事件 
	 * @param conn 连接对象
	 * @retval true 可处理
	 * @retval false Dispatcher已经被销毁
	 */
	bool handleConnLinkError( CAsyncConnection* conn );

	/**
	 * @brief 处理connection连接关闭事件 
	 * @param conn 连接对象
	 * @retval true 可处理
	 * @retval false Dispatcher已经被销毁
	 */
	bool handleConnClosed( CAsyncConnection* conn );
	
private:
	/**
	 * @brief 处理连接状态的封装函数
	 * @param conn 连接对象
	 * @param handler 实际处理的函数
	 * @retval true 可处理
	 * @retval false Dispatcher已经被销毁
	 */
	bool handleConn( CAsyncConnection* conn, Handler handler );

private:
	Infra::CMutex m_mutex; ///< 配合m_destroy保证销毁后不会再执行handler
	bool m_destroy; ///< true时说明已经调用了destroy()
	Handler m_connected_handler; ///< 连接成功处理者
	Handler m_error_handler; ///< 连接失败或异常处理者
	Handler m_close_handler; ///< 连接关闭处理者
	int32_t m_execute_thread_id; ///< 正在执行handleConn簇函数的的线程ID
	int32_t m_wait_num; ///< 等待handler处理结束的线程个数
	Infra::CSemaphore m_sem; ///< 用于等待handler处理结束
};

/// @brief CDispatcher智能指针
typedef Memory::TSharedPtr<CDispatcher> DispatcherPtr;

}
}

#endif
