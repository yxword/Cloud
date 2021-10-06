//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_SERVICEBASE_H__
#define __INCLUDE_DAHUA_EFS_SERVICEBASE_H__

#include "Infra/Mutex.h"
#include "Stream.h"
#include <map>

namespace Dahua {
namespace EFS {

class CServiceBase
{
public:
	CServiceBase();
	virtual ~CServiceBase();	

	//会话初始化
	bool init( NetFramework::CSockStream& sock );

	//请求req，回复rsp
	typedef void (CServiceBase::*Method)( TPacketBasePtr req, TPacketBasePtr& rsp ); 
	
	//添加方法处理接口
	//参数：type：消息类型
	//cmd：消息方法
	//method：消息处理函数
	void addMethod( uint8_t type, uint16_t cmd, Method method );

	//数据发送，一般情况可通过Method的rsp回复
	int send( TPacketBasePtr req );

	//建立定时器，精度为500毫秒，返回定时器ID
	int32_t setTimer( int32_t timeout );

	bool destroyTimer( int32_t id );

	bool setSendBufSize( uint32_t size );
	bool setRecvBufSize( uint32_t size );

	virtual void handleTimeout( int32_t id ){}

	//异常处理函数，fd异常时，此虚接口会执行
	virtual void handleException() = 0;

	//销毁接口，子类通过此接口发起对象销毁动作
	//最终引起handleClose被调用
	virtual void close();

	//对象最终销毁前执行此接口，只有此接口内才允许调用delete this，否则会有问题
	virtual void handleClose() = 0;
private:
	struct Internal;
	struct Internal*	m_internal;
};

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_SERVICEBASE_H__
