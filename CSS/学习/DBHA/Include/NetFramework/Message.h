//
//  "$Id: Message.h 26645 2011-05-31 00:35:01Z wang_haifeng $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-8-12		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_MESSAGE_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_MESSAGE_H__

namespace Dahua{
namespace NetFramework{
	
//来自NetFramework模块的消息，都以0x0XYZ表示，即小于十六进制数0x1000

//所有继承自CNetHandler类的类在自己的handle_close函数中都可以发送该消息
//表示自己的生命周期已经结束。消息的附带参数attach未使用。
#define MSG_OBJECT_CLOSED		0x0001
//发向调用者的消息，指示socket有异常，导致无法正常发送。
//消息的附带参数attach未使用。
#define MSG_SOCK_EXCEPTION	0x0002
//由class CStreamSender发向调用者的消息，指示当前空闲缓冲区的大小。
//消息的缓带参数attach即为空闲缓冲区大小。
#define MSG_BUFFER_SIZE			0x0004
//由CMediaStreamReceiver子类调用StopReceive、接收异常或直接调用Close后，框架发送至此子类的消息，表示成功停止接收
//消息的附带参数attach未使用。
#define MSG_TIMER_STOP			0x0005
//由CNFile类发向调用者的消息，指示当前文件可写
//消息的附带参数attach为空闲缓冲区大小
#define MSG_FILE_WRITE			0x0006
//由CNFile类发向调用者的消息，指示当前文件可读
//消息的附带参数attach为可读的数据量大小
#define MSG_FILE_READ			0x0007
//由CNFile类发向调用者的消息，表示文件读写出现异常。
//消息的附带参数attach未使用。
#define MSG_FILE_EXCEPTION	0x0008
//由class CStreamSender发向调用者的消息，指示当前缓冲区数据已经发送完毕
//消息的附带参数attach未使用。
#define MSG_SEND_FINISH		0x0009

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_MESSAGE_H__
