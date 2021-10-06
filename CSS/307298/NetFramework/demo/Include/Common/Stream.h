//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_STREAM_H__
#define __INCLUDE_DAHUA_EFS_STREAM_H__

#include "Infra/Function.h"
#include "NetFramework/SockAddr.h"
#include "NetFramework/SockStream.h"
#include "PacketBase.h"

namespace Dahua {
namespace EFS {

//协议传输类，基于TCP
class CStream
{
	CStream& operator=(CStream const& other);
	CStream(CStream const& other);	
public:
	CStream();
	~CStream();	
	enum
	{
		connected = 0,	//连接成功
		pktArrived,		//请求到达，可以是Request、Notify，不包括Response
		connError,		//连接异常		
		closed			//已经关闭，不再可能有回调上来，可以destroy
	};
	//参数1：状态
	//参数2：请求包
	typedef Infra::TFunction2<void, int, TPacketBasePtr> StateProc;		
	
	//请求回复到达回调
	//参数1：请求包
	//参数2：应答包
	typedef Infra::TFunction2<void, TPacketBasePtr, TPacketBasePtr> RspProc;	

	// Notify到达回调，需要先调用attachNotifyProc进行注册，一般在请求订阅得到确认之后
	// 参数1：请求到达notify消息
	typedef Infra::TFunction1<void, TPacketBasePtr> NotifyProc;
	
	// 会话初始化回调，在注册此回调之后，内部TCP连接成功之后，会通过这个回调和使用类交互，
	// 获取会话初始化必须的请求和返回结果，并通过回调返回true表示会话初始化完成
	typedef Infra::TFunction2<bool, int, TPacketBasePtr&> SessionInitProc;
	
	// 设置自动重连，一旦调用此接口，内部会自动触发异常之后的重连，第一次连接依然有connect发起,
	// 如果已经设置过，则StateProc回调的connected、connError外部不需要特殊处理
	// times：异常重试次数，0表示不重试，-1表示永久重试，正数表示次数
	// getReq：重试成功之后通过此回调获取业务初始化请求
	// callbackRsp：返回请求结果给外部
	bool setAutoReconnect( int32_t times, SessionInitProc getReq, SessionInitProc callbackRsp );
	
	// 设置连接参数，默认非阻塞，超时2秒
	// timeout：超时时间,单位毫秒
	// block：是否阻塞，设置为阻塞时，connect接口调用是阻塞的，
	// StateProc不会有状态为connected的回调和握手失败的connError上来
	bool setConnectOption( uint32_t timeout, bool block = false );

	bool setSendBufSize( uint32_t size );
	bool setRecvBufSize( uint32_t size );
		
	//发起或使用已有连接，状态变更时回调被调用
	int connect( StateProc proc, NetFramework::CSockAddr* remote, NetFramework::CSockAddr* local = NULL );

	// 注册某个cmd的notify到达处理函数，一般用于客户端
	// 需要注意的是，使用同步调用进行订阅的时候，应该先调用此接口注册，
	// 再发起订阅请求，避免消息丢失
	int attachNotifyProc( int16_t cmd, NotifyProc proc );
	// 注销某个cmd的notify到达处理函数，一般用于客户端
	int detachNotifyProc( int16_t cmd );
	
	//发送数据，不期望回复，用于回复Response或发送Notify
	//返回值：0：暂时不可写，-1：失败，>0：成功
	int send( TPacketBasePtr pkt );
	
	//发起请求，等待回复回调
	//pkt：请求消息
	//proc：回复回调
	//timeout：超时时间，单位毫秒，内部以500毫秒计算
	//返回值：0：暂时不可写，-1：失败，>0：成功
	int call( TPacketBasePtr pkt, RspProc proc, uint32_t timeout );
	// 发起请求，阻塞等待回复
	// req：请求消息
	// rsp：回复结果
	// timeout：超时时间，单位毫秒
	// 返回值：0成功，-1失败
	int call( TPacketBasePtr req, TPacketBasePtr* rsp, uint32_t timeout );
	
	//请求关闭，若之前有call没有返回，会触发回调回来，直到最后回调closed
	void close(); 
	
	// 重置连接，会触发关闭fd，后续逻辑根据是否重连而不同
	void reset();
private:
	class CStreamCore;
	CStreamCore*	m_internal;
};

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_STREAM_H__
