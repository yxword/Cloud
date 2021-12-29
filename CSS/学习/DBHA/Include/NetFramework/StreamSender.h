//
//  "$Id: StreamSender.h 30412 2011-07-21 05:57:44Z qin_fenglin $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-7-2		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_STREAM_SENDER_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_STREAM_SENDER_H__

#include "Infra/IntTypes.h"
#include "NetHandler.h"
#include "Sock.h"
#include "Message.h"
#include "NDefs.h"

namespace Dahua{
namespace NetFramework{

//普通（非流媒体）TCP流数据发送者。
//注意：CStreamSender必须通过通过其静态成员Create操作被创建出来，否则无法正常工作。
//特别注意：CStreamSender中虽然调用了Attach绑定了socket对象，但socket对象的真正属
//主还是外部的调用者，所以请在你自己的代码中实现如下的handle_output函数：
//		int handle_output( int handle )
//		{
//				return CStreamSenderObj->handle_output( handle );
//		}
class NETFRAMEWORK_API CStreamSender : public CNetHandler
{
	CStreamSender& operator=( CStreamSender const& other );
	CStreamSender( CStreamSender const& other );
public:
	//创建一个CStreamSender对象，
	//返回值： 成功则返回一个CStreamSender对象，否则返回NULL。
	static CStreamSender * Create();
	//输入待发送数据，这些数据由CStreamSender负责发送，调用者不必关心。
	//参数： buf: 待发送的数据。
	//			 len: 待发送数据的长度。
	//返回值：0成功，-1失败。
	int Put( const char * buf, uint32_t len );
	//绑定一个tcp socket对象，数据将在这个socket上发送。绑定之后，
	//就不可以在其它地方再对这个socket进行数据发送操作，但读操作和其它操作可正常进行。
	//参数： stream: 待绑定的stream对象。
	//返回值：0成功，-1失败。
	int Attach( CNetHandler *owner, CSock * stream );
	//解绑定一个tcp socket对象，解绑定之后，CStreamSender失去发送数据的能力。
	//返回值：被解绑定的stream对象。
	CSock * Detach();
	//设置CStreanSender的发送缓冲区的大小，缺省为256K，也即8个32K大小。
	//参数： size_in_32K：缓冲区的大小，以32K为单位，1表示32K，2表示64K。
	//返回值：0成功，-1失败。
	int SetBufferSize( uint32_t size_in_32K );
	//获取CStreamSender的发送缓冲区的大小。
	//返回值：当前缓冲区大小，以32K为单位。
	uint32_t GetBufferSize();
	//清空CStreamSender的缓冲区。
	//返回值：0
	int Clear();
	//调用者通过这个接口，告诉CStreamSender下一次Put调用需要的缓冲区大小，
	//当CStreamSender的缓冲区达到要求时，就会向调用者发送消息。
	// 参数： receiver_id：消息的接收者，通常就是调用者。
	//				size_in_byte: 调用者需要等待的空闲缓冲区的大小，以字节为单位。
	//注意：可有有多个调用者要求这个消息。
	void WaitBufferSize( long receiver_id, uint32_t size_in_byte );
	//调用者通过这个接口，可以获得已经put进去的数据完整发完消息
	void WaitSendFinish( long receiver_id );
	//调用者通过这个接口，告诉CStreamSender，如果socket发生异常，就需要
	//向调用者发送消息。
	//参数： receiver_id: 消息的接收者，通常就是调用者。
	//注意：只能有一个调用者要求这个消息。
	void WaitException( long receiver_id );	
private:
	struct Internal;
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_STREAM_SENDER_H__
