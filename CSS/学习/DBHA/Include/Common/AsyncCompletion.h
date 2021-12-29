#ifndef __INCLUDE_DAHUA_EFS_ASYNC_COMPLETION__H__
#define __INCLUDE_DAHUA_EFS_ASYNC_COMPLETION__H__

#include "Infra/Function.h"
#include "Infra/IntTypes.h"
#include "Infra/Mutex.h"
#include "Infra/Semaphore.h"
#include "Memory/SharedPtr.h"

#include <list>

namespace Dahua{
namespace EFS{
	
/**
 * @brief 感知异步操作完成类
 * @details 支持同步等待和回调通知两种方式，如果两者同时使用顺序是先执行回调函数,再通知等待返回。
 * @note complete()可以被多次调用，但是只有第一次调用的结果有效
 * @warn 因为先执行回调，再通知等待返回，如果等待是在某一把锁内，而回调函数需要进入相同的锁，此时会形成死锁。
 * @warn 不得在注册的回调函数中调用该对象的waitForComplete()
 */
class CAsyncCompletion
{
public:
	/**
	 * @brief 完成时的回调函数类型
	 * @note 参数1为执行结果，参数2为指定的回调参数
	 */
	typedef Infra::TFunction2<void, int32_t, void*> Callback;
public:
	CAsyncCompletion();
	~CAsyncCompletion();

	/**
	 * @brief 设置完成时的回调函数及其参数
	 * @param cb 回调函数
	 * @param arg 回调函数的参数
	 * note 因为指针的大小各平台不同，所以如果传递非指针的数据，只允许传递32位以下的数值，避免溢出
	 */
	void setCallback( Callback cb, void* arg );

	/**
	 * @brief 等待完成
	 */
	void waitForComplete();

	/**
	 * @brief 获取结果
	 * @return 返回值与实际操作有关
	 */
	int32_t getResult();
	
	/**
	 * @brief 重置该对象，使对象可再次被使用
	 * @note 调用后，对象为未完成状态，所有注册函数清空
	 * @warn 只有CAsyncCompletionMgr调用
	 */
	 void reset();

	/**
	 * @brief 完成调用该函数
	 * @param result 操作执行结果
	 */
	void complete( int32_t result );

	/**
	 * @brief 取消等待
	 * @note 由异步操作的调用方使用，不会触发注册的回调，如果有wait则会结束wait
	 * @note 调用后，对象为已完成状态，所有注册函数清空，getResult()将返回-1
	 * @note 只在对象为未完成状态时有效，否则不做任何操作
	 */
	 void cancel();
private:
	Infra::CMutex m_mutex;///< 保护以下临界资源
	bool m_complete; ///< true已完成，false未完成
	bool m_first_complete; ///< 第一次执行complete时，才更新结果
	int32_t m_result;///< 完成的结果
	
	int32_t m_wait_complete;///< 等待的个数
	Infra::CSemaphore m_sem;///< 用于同步等待的信号量
	
	Callback m_callback; ///< 外部注册的回调函数
	void* m_callback_arg; ///< 外部注册的回调函数的参数
};

/// @brief CAsyncCompletion智能指针
typedef Memory::TSharedPtr<CAsyncCompletion> AsyncCompletionPtr;

/**
 * @brief 管理进程中使用到的CAsyncCompletion
 * @details CAsyncCompletion会被频繁使用，如果反复申请释放，会存在内存碎片。
 *	做法：一次性申请一大段内存，并划分为多个CAsyncCompletion，将该内存划给AsyncCompletionPtr管理时指定删除函数，
 *	该删除函数内不释放内存,而是将内存重新赋给新的AsyncCompletionPtr，并加入可用队列。
 *	内存统一由CAsyncCompletionMgr析构时释放
 */
class CAsyncCompletionMgr
{
public:
	///	@brief AsyncCompletionPtr智能指针释放内存的函数
	typedef Infra::TFunction1<void, CAsyncCompletion*> FreeFunc;
		
public:
	static CAsyncCompletionMgr* instance();
	CAsyncCompletionMgr();
	~CAsyncCompletionMgr();

	/**
	 * @brief 获取可用的AsyncCompletionPtr
	 * @return 可用的AsyncCompletionPtr
	 * @note 该函数保证获取到的AsyncCompletionPtr不为NULL
	 */
	AsyncCompletionPtr getAsyncCompletion();

private:
	/// @brief 创建可用的AsyncCompletion
	void createAsyncCompletions();

	/// @brief 释放空闲的AsyncCompletion
	void freeAsyncCompletions();
	
	/// @brief AsyncCompletionPtr管理的内存的删除函数
 	void deleteAsyncCompletionPtr( CAsyncCompletion* p );
	
private:
	Infra::CMutex m_mutex; ///< 保护m_async_completions
	std::list<CAsyncCompletion*> m_raw_completions; ///< 原始内存CAsyncCompletion
	std::list<std::list<AsyncCompletionPtr> > m_idle_completions;///< 可用的AsyncCompletionPtr，每个list对应m_raw_completions的一个元素
};

#define gAsyncCompletionMgr Dahua::EFS::CAsyncCompletionMgr::instance()

}
}

#endif
