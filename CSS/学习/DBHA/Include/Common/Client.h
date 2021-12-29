#ifndef __INCLUDE_DAHUA_EFS_BASE_CLIENT__H__
#define __INCLUDE_DAHUA_EFS_BASE_CLIENT__H__

#include "Dispatcher.h"
#include "Operation.h"
#include "AsyncConnection.h"

#include "Common/StructBase.h"
#include "Common/Timer.h"
#include "Infra/IntTypes.h"
#include "Infra/Mutex.h"
#include "Infra/ReadWriteMutex.h"

namespace Dahua{
namespace EFS{

class CAsyncConnection;
class CAsyncMessenger;
/**
 * @brief 客户端类，实现收发消息的基本功能
 * @details connect()连接服务，
 *          sendMsg()发送消息，
 *          通过OperationPtr::finish()处理响应结束一次操作
 *          可设置消息超时时间，支持响应超时处理
 * @note 由子类按需实现重连功能
 * @note 如需设置参数，那么必须在使用CClient对象前调用set*()接口
 * @warn 一旦调用destroy()后，对象不可再使用
 */
class CClient
{
public:
	CClient( CAsyncMessenger* messenger );
	virtual ~CClient();

	/**
	 * @brief 根据指定的ip和port连接服务
	 * @param ip 服务ip地址
	 * @param port 服务端口
	 * @retval efsSucceed 成功
	 * @retval 其他 失败，详见ErrorCode
	 * @note 外部保证单线程调用
	 */
	int32_t connect( const char* ip, uint16_t port );
	
	/**
	 * @brief 根据指定的ip和port连接服务
	 * @param ip 服务ip地址
	 * @param port 服务端口
	 * @retval efsSucceed 成功，等待complete
	 * @retval 其他 失败，详见ErrorCode
	 * @note 外部保证单线程调用
	 * @warn complete可能在本次调用时就完成回调，需要注意，避免死锁
	 */
	int32_t asyncConnect( const char* ip, uint16_t port, AsyncCompletionPtr complete );
	
	/**
	 * @brief 重置连接
	 * @note 当对端设备突然挂掉时，内核可能无法立即感知到TCP链路断开，上层可依赖心跳超时来主动调用该接口去重置链路
	 */
	void reset();
	
	/**
	 * @brief 销毁内部申请的资源
	 * @note 外部保证单线程调用
	 */
	void destroy();

	/**
	 * @brief 发送消息接口
	 * @param 组装好的消息对象
	 * @param op 消息对应的操作，主要用于处理响应，传递OperationPtr()
	 * @retval efsSucceed 发送成功
	 * @retval efsNetworkAgain 重试
	 * @retval efsNetworkError 网络错误，需要destroy()后重新connect()
	 */
	int32_t sendMsg( TPacketBasePtr msg, OperationPtr op );

	/**
	 * @brief 设置消息超时时间
	 * @param timeout 超时时间，单位毫秒，取值范围[1000, ...)或-1永不超时
	 * @note 默认为2000毫秒
	 */
	void setTimeout( int32_t timeout );

	/**
	 * @brief 设置链路复用
	 * @param reuse true复用链路，false不复用链路
	 * @note 默认不复用链路，必须在connect之前调用
	 * @warn 目前不支持链路复用，2017.11.3
	 */
	void setReuseConnection( bool reuse);

	/**
	 * @brief 获取远端地址
	 * @param ip 远端ip
	 * @param port 远端port
	 */
	void getRemoteAddr( std::string& ip, uint16_t& port );
protected:
	/**
	 * @brief 触发重连
	 * @note 在处理链路错误时，首次触发。后续根据reconnect()结果判断是否触发
	 * @note 当子类重连失败时可以调用
	 */
	void triggerReconnect();
	
private:

	/**
	 * @brief 重连操作回调函数
	 * @param result 为符合回调函数类型，实际无用
	 * @param arg 为符合回调函数类型，实际无用
	 * @note 
	 */
	void _reconnect( int32_t result, void* arg );
	
	/**
	 * @brief 网络异常后重连
	 * @retval efsSucceed 成功
	 * @retval 其他 失败
	 * @note 失败情况下，CClient会再次触发重连。若无需重连请返回efsSucceed
	 * @warn 考虑到使用的定时器是EFS::CTimer而非独立线程，该函数必须是非阻塞实现，否则可能阻塞该定时器中的其他操作
	 * @warn 因为在EFS::CTimer线程中执行detach()动作时将陷入死循环，所以不得在重连中调用CClient::destroy()函数。
	 */
	virtual int32_t reconnect();

	/// @brief 处理网络连接成功事件
	void handleConnConnected( CAsyncConnection* conn );
	/// @brief 处理网络连接错误事件
	void handleConnLinkError( CAsyncConnection* conn );
	/// @brief 处理网络连接关闭事件
	void handleConnClosed( CAsyncConnection* conn );
	/// @brief 操作结束时回调函数
	void finishOpCallback( int32_t result, void* arg );

private:
	CAsyncMessenger* m_messenger; ///< 添加到该messenger
	DispatcherPtr m_dispatcher; ///< 处理连接事件，构造时申请初始化
	
	Infra::CMutex m_conn_mutex; ///< 保护m_conn的使用
	AsyncConnectionPtr m_conn; ///< 使用的网络连接
	std::string m_remote_ip; ///< m_conn应该连接的远端地址
	uint16_t m_remote_port; ///<  m_conn应该连接的远端端口
	
	bool m_reuse_conn; ///< true复用链路，false不复用链路。

	Infra::CMutex m_op_mutex; ///< 保护以下字段
	int32_t m_op_timeout; ///< 操作超时时间，单位毫秒
	OperationPtr m_connect_op; ///< 连接操作	
	OperationPtr m_reconnect_op; ///< 重连操作,通过operation超时来实现定时重连
	std::map<void*, OperationPtr> m_unfinished_ops; ///< key为op的地址
	bool m_connected_once; ///< true表示已经连接成功过，主要作为是否触发内部重连的判断标记

	Infra::CReadWriteMutex m_destroy_mutex; ///< 读锁锁定对成员变量的操作，写锁保护m_destroy的修改
	bool m_destroy; ///< true代表已经调用destroy()，对象不可再使用
};

}
}
#endif
