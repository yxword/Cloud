//
//  "$Id: NetHandler.h 30412 2011-07-21 05:57:44Z qin_fenglin $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-11		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_NET_HANDLER_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_NET_HANDLER_H__

#include "Infra/IntTypes.h"
#include "NDefs.h"
#include "Sock.h"

namespace Dahua{
namespace NetFramework{

typedef unsigned int MSG_TYPE;
typedef unsigned int SOCK_MASK;
enum{
	NONE_MASK = 0,					//无事件。
	READ_MASK = (1 << 0),
	WRITE_MASK = (1 << 1),
	EXCEPT_MASK = (1 << 2),
	TIMEOUT_MASK = (1 << 3),		//超时事件。
};

class NETFRAMEWORK_API CNetHandler
{
	CNetHandler& operator=( CNetHandler const & other );
	CNetHandler( CNetHandler const & other );
public:
	CNetHandler();
	//私有线程池中CNetHandler对象构造函数。
	//参数prv_idx为CNetHandler对象所在的私有线程池索引。
	CNetHandler( int32_t prv_idx );
	virtual ~CNetHandler();
protected:
	//向线程注册socket，一个socket可以同时注册读和写事件的。
	int RegisterSock( CSock & socket, SOCK_MASK type, int timout = 0 );
	int UnregisterSock( CSock & socket, SOCK_MASK type );
	//删除注册的socket
	int RemoveSock( CSock & socket );

	//创建一个定时器，单位为微秒(百万分之一秒)
	//参数为定时器的下一个超时时间，单位为百万分之一秒。
	//返回值为定时器的ID号。
	long SetTimer( int64_t usec );
	//销毁定时器，参数为SetTimer返回的值。
	void DestroyTimer( long id );

	//向另一个CNetHandler对象传递异步消息，消息接收顺序不确定
	//参数：receiver_id为接收者的ID,type为消息类型。info为附带消息。
	void Notify( long receiver_id, MSG_TYPE type, long info );

	//向事先通过SetDefaultRcer设置的固定接收者，传递异步消息，不必显式指定接收者。
	void Notify( MSG_TYPE type, long info );

	//串行化地向另一个CNetHandler对象传递消息，先发送的消息会先被处理处理
	// \param[in] receiver_id 接收者的ID
	// \param[in] type 消息类型
	// \param[in] info 附带消息
	// \param[in] flag 保留参数
	void NotifySerial( long receiver_id, MSG_TYPE type, long info, unsigned int flag = 0 );
	void NotifySerial( MSG_TYPE type, long info, unsigned int flag = 0);

	//如果消息的接收者在处理该消息时，需要进行设备I/O或其它较为耗时的操作，则调用这两个接口
	//参数含义同上。
	void NotifyIO( long receiver_id, MSG_TYPE type, long info );
	void NotifyIO( MSG_TYPE type, long info );

	//内存的申请和释放。这是线程缓冲版本，但必须在NetFramework的线程中调用。
	void* TsMalloc( uint32_t size );
	void TsFree( void* ptr );
	//内存的申请和释放，这是非线程缓冲版本，使用更为灵活，任何线程中都可以调用，但效率有所下降。
	void* Malloc( uint32_t size );
	void Free( void* ptr );
public:
	//通过该接口，可以设置一个对象为本对象的默认消息接收者。
	void SetDefaultRcer( long receiver_id );
	//返回对象的ID。
	long	GetID();

public:
	//网络事件对应的回调函数，具体是属于哪个socket上的事件，在这些函数体中再
	//通过handle来区分。
	//返回-1：取消socket上的等待超时设置。
	//返回0:维持原有的等待超时设置。
	//返回大于0：表示下一个等待超时时间（单位为百万分之一秒）。
	virtual int handle_input( int handle ) { return -1; }
	virtual int handle_output( int handle ) { return -1; }
	//为安全起见，强烈建议在你的派生类中重新实现该函数，否则有可能导致整个系统无响应！！
	virtual int handle_exception( int handle ) { return -1; }

	virtual int handle_input_timeout( int handle ) { return -1; }
	virtual int handle_output_timeout( int handle ) { return -1; }
	virtual int handle_exception_timeout( int handle ) { return -1; }
	//定时器超时的回调函数，单位为SetTimer返回的值。
	//返回-1：销毁该定时器.
	//返回0：维持原有的定时器设置.
	//返回大于0的值，表示下一个超时时间。
	virtual int64_t handle_timeout( long id ) { return -1; }
	//CNetHandler对象的析构处理函数，只有在这个函数中才允许delete this指针，否则会造成问题。
	virtual int handle_close( CNetHandler* myself )	{ return -1; }
	//消息处理函数，from是消息的发送者，type是消息类型，attach是消息的附带参数.
	//消息的类型和附带参数由发送者和接收者之间约定，框架不作任何约束。
	virtual int handle_message( long from_id, MSG_TYPE type, long attach ) { return -1; }

	//销毁对象，凡是继承了CNetHandler的对象，必须通过这个接口销毁自己，销毁其它CHanddler对象
	//也必须调用该接口，然后在回调函数hanlde_close()中处理相关的销毁事宜。
	//调用后会触发handle_close的异步回调，由于触发回调时间不确定，不要在Close之后操作NetHandler对象
	virtual void Close();

private:
	struct Internal;
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_NET_HANDLER_H__
