#ifndef __INCLUDE_DAHUA_EFS_ASYNC_OPERATION__H__
#define __INCLUDE_DAHUA_EFS_ASYNC_OPERATION__H__

#include "AsyncCompletion.h"

#include "Common/PacketBase.h"

#include "Infra/IntTypes.h"
#include "Infra/Mutex.h"
#include "Memory/SharedPtr.h"

namespace Dahua{
namespace EFS{
	

/**
 * @brief 操作基类
 * @details 为支持多种消息的处理，提供虚函数msgfinish()由子类实现。
 *			正常结束时调用finish()，需要中断时调用destroy()。
 *			一旦调用destroy()，如果正在finish()中，则会等待finish()调用结束，否则直接返回，且之后调用finish()将不执行任何操作。
 *			如果是在finish()执行的操作中调用了destroy()，则依赖递归锁保证不出现死锁。
 * @note 推荐以智能指针OperationPtr的方式使用
 * @warn 不支持多线程调用同一接口
 */
class COperation
{
public:
	/**
	 * @param c 操作结束时的通知对象。不关心操作是否完成时，设为NULL
	 */
	COperation( int32_t type, AsyncCompletionPtr c );
	virtual ~COperation();

	/**
	 * @brief 操作开始
	 * @note 操作开始时调用一次，主要是记录开始时间
	 */
	void start();

	/**
	 * @brief 通用结束操作
	 * @param result 操作执行结果
	 * @note 操作按预定流程结束时使用
	 */
	void finish( int32_t result );
	
	/**
	 * @brief 以处理消息作为操作结束标识专用
	 * @param msg 响应消息
	 * @note 需子类去理解msg的含义,最后仍需调用finish( int32_t result )完成整个操作
	 */
	virtual void msgfinish( TPacketBasePtr msg );
	
	/**
	 * @brief 设置操作超时时间
	 * @param timeout 超时时间，单位毫秒
	 */
	void setTimeout( int32_t timeout );
	
	/**
	 * @brief 检查是否超时
	 * @retval true 已超时
	 * @retval false 未超时
	 */
	bool isTimeout();

	/**
	 * @brief 获取操作类型
	 * @return 返回操作类型
	 */
	int32_t type();

	/**
	 * @brief 增加扩展的completion
	 * @param c 操作结束时的通知对象
	 * @note 为CClient的使用特意新增。后续考虑更合理的方式
	 */
	void addExtCompletion( AsyncCompletionPtr c );
private:
	Infra::CMutex m_mutex; ///< 保护如下操作
	AsyncCompletionPtr m_completion; ///< 操作结束时的通知对象
	AsyncCompletionPtr m_ext_completion; ///< 扩展的操作结束时的通知对象
	int32_t m_wait_num; ///< 等待complete()结束的线程个数
	int32_t m_execute_thread_id; ///< 正在执行complete()的线程ID
	Infra::CSemaphore m_sem; ///< 用于等待complete()结束。
	
	uint64_t m_start_time;///< 单位毫秒
	int32_t m_timeout; ///< 单位毫秒
	int32_t m_type; ///< 操作类型，使用者定义

};

/// @brief AsyncOperation智能指针
typedef Memory::TSharedPtr<COperation> OperationPtr;

}
}

#endif
