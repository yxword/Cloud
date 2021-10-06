//
//  "$Id: MediaStreamReceiver.h 30412 2011-07-21 05:57:44Z qin_fenglin $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-7-9		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_MEDIA_STREAM_RECEIVER_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_MEDIA_STREAM_RECEIVER_H__

#include "Infra/IntTypes.h"
#include "NDefs.h"
#include "NetHandler.h"
#include "Message.h"

namespace Dahua{
namespace NetFramework{
	
//流媒体接收工具，使用该工具需要继承它，并实现纯虚函数handle_stream_input。
class NETFRAMEWORK_API CMediaStreamReceiver : public CNetHandler
{
	CMediaStreamReceiver& operator=( CMediaStreamReceiver const & other );
	CMediaStreamReceiver( CMediaStreamReceiver const & other );
public:
	CMediaStreamReceiver();
	~CMediaStreamReceiver();
public:
	//设置异常消息的接收者。
	//参数：receiver_id: 消息的接收者。
	//设置了接收者，如果socket接收发生异常，该接收者就会收到消息MSG_SOCK_EXCEPTION
	//如果调用了StopReceive、接收异常、或者调用了Close后，该接收者会收到消息MSG_TIMER_STOP
	void WaitException( long receiver_id );
	//设置本次接收的流媒体的帧率。
	//参数：fps：流媒体的帧率。
	//此接口已废弃
	void SetFrameRate( uint32_t fps );
	//启动接收流程，上述两个参数设置完成后，即可启动接收。
	//参数：fast_start_ts：快速启动时间
	//		rate：播放速度
	void StartReceive( int64_t fast_start_ts = 0, float rate = 1.0 );
	//暂停接收。
	void StopReceive();
	//重载Close
	void Close();
	//设置Receiver的工作模式，不设置则默认为主动模式
	typedef enum
	{
		INITIATIVE_RECEIVER = 0, //主动模式，如网络接收，默认为主动模式
		PASSIVE_RECEIVER = 1,	//被动模式，如文件 
		DISK_WRITE = 2
	}receiver_mode;
	void SetReceiveMode( receiver_mode mode );	
	//需要用户自己实现的接收函数，在该函数中，从socket上接收数据，
	//返回值：本次实际收到的完整帧数。
	//				接收失败，则返回-1。
	virtual int handle_stream_input(){ return -1; }
	//需要用户自己实现的被动接收函数，在该函数中，从文件上读取一个完整媒体帧
	//返回值：本帧的pts
	//读取失败，则返回-1。
	virtual int64_t handle_passive_input(){ return -1; }
	//需要用户自己实现的写入函数，在该函数中，用户执行相应的I/O操作，
	//返回值：大小等于0，表示成功。
	//				失败返回-1。
	virtual int handle_stream_write() { return -1; }
private:
	struct Internal;
	struct Internal*	m_internal;
};


}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_MEDIA_STREAM_RECEIVER_H__
