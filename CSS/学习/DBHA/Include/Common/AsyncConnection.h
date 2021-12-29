#ifndef __INCLUDE_DAHUA_EFS_ASYNC_CONNECTION__H__
#define __INCLUDE_DAHUA_EFS_ASYNC_CONNECTION__H__

#include "Dispatcher.h"
#include "Operation.h"

#include "Common/PacketBase.h"
#include "Infra/AtomicCount.h"
#include "Infra/IntTypes.h"
#include "Infra/Mutex.h"
#include "Memory/SharedPtr.h"

#include <string>
#include <list>
#include <map>

namespace Dahua{
namespace EFS{

class CConnection;
class CAsyncMessenger;
/**
 * @brief 异步网络连接对象 
 * @details 对CConnection的封装，所有操作均为异步。
 *		    内部保证消息序列号唯一，序列号从1开始。提供响应超时处理功能。
 * @note CAsyncConnection是加锁使用CDispacther的，为避免出现ABBA死锁，
 *		 外部需在调用CAsyncConnection接口时，不会锁定CDispacther回调函数中要锁定的锁
 * @note 因为CConnection的特性，一旦构造该类对象后，不论是否connect，销毁前必须调用close()，触发连接关闭事件回调。
 */
class CAsyncConnection
{
private:
	enum ConnStatus
	{
		CONN_INIT, ///< 初始状态
		CONN_ESTABLISHED, ///< 已连接
		CONN_ERROR, ///< 错误
		CONN_CLOSED, ///< 已关闭
	};

public:
	/**
	 * @param messenger 所属的CAsyncMessenger地址
	 * @param id NetFramwork的私有线程池ID
	 */
	CAsyncConnection( CAsyncMessenger* messenger, int32_t id = -1 );
	~CAsyncConnection();

	/**
	 * @brief 发起网络连接
	 * @param ip 远端地址
	 * @param port 远端端口
	 * @retval efsSucceed 成功
	 * @retval efsFail 失败
	 * @note 只在对象构造后调用一次
	 */
	int32_t connect( const char* ip, uint16_t port );

	/**
	 * @brief 发送消息
	 * @param m 消息对象
	 * @param op 消息对应的操作，用于响应处理,为空说明无需处理响应
	 * @retval efsSucceed 发送成功
	 * @retval efsNetworkAgain 暂时无法发送
	 * @retval efsNetworkError 发送失败
	 * @note 内部设置m的序列号
	 */
	int32_t send( TPacketBasePtr m, OperationPtr op );

	/**
	 * @brief 关闭网络连接
	 * @note 只在发生错误或无人使用后调用一次
	 */
	void close();

	/**
	 * @brief 返回远端IP
	 * @return 返回远端IP
	 */
	std::string getRemoteIp();

	/**
	 * @brief 返回远端端口
	 * @return 返回远端端口
	 */
	uint16_t getRemotePort();

	/**
	 * @brief 将dispatcher挂到该连接上，处理连接事件
	 * @param dispatcher 处理conn事件的dispatcher
	 * @note 可能在调用时触发dispatcher的回调处理，注意不要产生死锁
	 */
	void attachDispatcher( DispatcherPtr dispatcher );
	
private:
	/**
	 * @brief 注册给CConnection的回调函数
	 * @param type 回调的事件类型
	 * @param msg 消息对象
	 * @note 当因连接事件回调时，msg为NULL
	 */
	void connCallBack( int type, TPacketBasePtr msg );

	/**
	 * @brief 处理连接状态
	 * @param status 状态
	 */
	void handleConnStatus( ConnStatus status );

	/**
	 * @brief 分发消息
	 * @param msg 消息体
	 */
	void handleMsg( TPacketBasePtr msg );
	
	/**
	 * @brief 清理超时的操作
	 * @param id 定时器ID
	 * @note 被EFS::CTimer每500ms调用一次
	 */
	void clearTimeoutOp( int64_t id );

private:
	CAsyncMessenger* m_messenger; ///< 对象所属的CAsyncMessenger
	Infra::CAtomicCount m_seq; ///< 消息序列号，从1开始，不考虑其溢出的情况
	CConnection* m_conn;///< 真实连接对象
	int64_t m_clear_timeout_op_timer_id; ///< 清理超时操作的定时器id
	
	// 外部保证单线程操作且不会改变，无需加锁
	std::string m_remote_ip; ///< 远端ip
	uint16_t m_remote_port; ///< 远端端口

	Infra::CMutex m_status_mutex; ///< 保护状态变化
	ConnStatus m_status; ///< 连接状态
	std::list<DispatcherPtr> m_dispatchers; ///< 状态发生变化时，将通过该list中所有dispatcher,复用m_status_mutex
	
	Infra::CMutex m_op_mutex; ///< 保护m_unfinished_ops
	std::map<uint64_t, OperationPtr> m_unfinished_ops; ///< 保存响应处理操作，key为消息序列号	
};

/// @brief AsyncConnection智能指针
typedef Memory::TSharedPtr<CAsyncConnection> AsyncConnectionPtr;

}
}
#endif
