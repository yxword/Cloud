#ifndef __INCLUDE_DAHUA_EFS_ASYNC_MESSENGER__H__
#define __INCLUDE_DAHUA_EFS_ASYNC_MESSENGER__H__

#include "AsyncConnection.h"

#include "Common/Defs.h"
#include "Infra/IntTypes.h"
#include "Infra/Timer.h"
#include "Infra/Mutex.h"
#include "Infra/Semaphore.h"
#include "Infra/ReadWriteMutex.h"

#include <map>
#include <string>
#include <list>

namespace Dahua{
namespace EFS{

/**
 * @brief CAsyncMessenger支持处理的超时操作
 */
enum TimeoutOpType
{
	CONNECT_OP, // 连接操作
	RECONNECT_OP, // 重连操作
};
	
/**
 * @brief 管理网络连接，处理网络连接事件
 * @details 提供定时器处理CClient的连接和重连等操作的超时处理，主要是因为这些操作超时可能会销毁CClient对象本身并注销定时器。
 *		    因为EFS::CTimer不支持在定时器处理线程中注销定时器，所以定时器必须由其他类提供，目前选择放在CMessenger类
 * @note 因为CAsyncMessenger使用了EFS::CTimer，所以必须保证CAsyncMessenger在EFS::CTimer单例析构前析构
 */
class CAsyncMessenger
{
public:
	/**
	 * @brief 构造函数
	 * @param id 指定NetFramwork的私有线程池id，不使用可不填写
	 */
	CAsyncMessenger( int32_t id = -1 );
	~CAsyncMessenger();

	/**
	 * @brief 获取网络连接
	 * @param ip 对端IP
	 * @param port 对端端口
	 * @param reuse 是否复用网络连接
	 * @return AsyncConnectionPtr对象智能指针，需检查是否为NULL，NULL为失败
	 */
	AsyncConnectionPtr getConnection( const char* ip, uint16_t port, bool reuse );

	/**
	 * @brief 归还网络连接
	 * @param conn 连接对象
	 * @note 内部会执行conn.reset()清空conn指向的地址
	 */
	void putConnection( AsyncConnectionPtr& conn );

	/**
	 * @brief 添加超时操作
	 * @param op 操作对象
	 * @note 只支持enum TimeoutOpType中的类型
	 */
	void addTimeoutOp( OperationPtr op );
	
	/**
	 * @brief 删除超时操作
	 * @param op 操作对象
	 */
	void delTimeoutOp( OperationPtr op );
	
	/** 
	 * @brief 处理连接成功事件
	 * @param conn 对象地址
	 * @note 只会被CAsyncConnection对象调用
	 */
	void handleConnConnected( CAsyncConnection* conn );
	
	/** 
	 * @brief 处理连接失败事件
	 * @param conn 对象地址
	 * @note 只会被CAsyncConnection对象调用
	 */
	void handleConnLinkError( CAsyncConnection* conn );
	
	/** 
	 * @brief 处理连接关闭事件
	 * @param conn 对象地址
	 * @note 只会被CAsyncConnection对象调用
	 */
	void handleConnClosed( CAsyncConnection* conn );

private:
	/// @brief 从已有连接中查找指定IP和PORT的conn对象
	AsyncConnectionPtr lookupConn( const char* ip, uint16_t port );
	/// @brief 根据IP和PORT创建conn对象
	AsyncConnectionPtr createConn( const char* ip, uint16_t port );
	/// @brief 定时处理超时的操作
	void timeoutOpTimer( int64_t id );
private:
	int32_t m_pri_thread_pool_id; ///< 使用的私有线程池ID，默认为-1

	// 因为触发conn的关闭到真正关闭有一个时间差，若在这时间差内，该conn还在m_conns中，则可能会被复用，
	// 所以先保存在m_close_conns中
	Infra::CMutex m_conns_mutex; ///< 保护m_conns的增删查
	std::multimap<std::string, AsyncConnectionPtr> m_conns;///< key为"ip:port", 当不复用链接时, 一个地址会有多个网络连接
	std::list<AsyncConnectionPtr> m_close_conns; ///< 需要关闭的conn会从m_conns中移入该list
	Infra::CSemaphore* m_wait_conn_close_sem;///< 用于析构时等待所有conn关闭，复用m_conn_mutex保证多线程安全

	int64_t m_timer_id; ///< 定时器ID
	Infra::CMutex m_timeout_op_mutex; ///< 保护m_timeout_ops
	std::map<void*, OperationPtr> m_timeout_ops; ///< 保存超时时需要执行的操作
};

}
}

#endif
