//
//  "$Id: MediaStreamSender.h 55559 2012-03-26 02:51:26Z zhou_mingwei $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-7-4		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_MEDIA_STREAM_SENDER_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_MEDIA_STREAM_SENDER_H__

#include "Infra/IntTypes.h"
#include "Infra/Function.h"
#include "Sock.h"
#include "NetHandler.h"
#include "Message.h"
#include "NDefs.h"
#include <string.h>

namespace Dahua{
namespace NetFramework{

class CMediaBuffer;
class CFrame;

//为每个帧定义级别，总共有5级，在丢帧时，级别低的优先丢弃。
#define LEVEL_NUM		10 //5 -> 10, resolve "rtp lost packet issue between MTS -> SS";
//输入的帧的级别，LEVEL_HIGH级别最高，LEVEL_LOW级别最低，总共LEVEL_NUM级。
#define LEVEL_HIGH	1
#define	LEVEL_LOW		LEVEL_NUM

//表示媒体数据包的虚基类，由用户实现数据的获取与释放。
class NETFRAMEWORK_API CMediaPacket
{
public:
	CMediaPacket(){}
	virtual ~CMediaPacket(){}
public:
	//释放该数据包。
	virtual void Release() = 0;
	//获取待发送的媒体数据包缓冲区的指针。
	virtual unsigned char *GetBuffer() = 0;
};

//流媒体的发送者，它绑有一个socket对象CSockStream，只要把这样的一个对象
//加入到一个流媒体缓冲区CMediaBuffer上，它就能通过socket把缓冲区上的流
//数据发送出去。
//注意： CMediaStreamSender必须通过其静态成员Create操作被创建出来，否则无法正常工作。
class NETFRAMEWORK_API CMediaStreamSender : public CNetHandler
{
	CMediaStreamSender( CMediaStreamSender const & other );
	CMediaStreamSender& operator=( CMediaStreamSender const & other );
public:
	//定义AttachSendProc要绑定的函数类型
	//参数： void          函数返回值
	//			 CMediaPacket* 帧信息包指针
 	//			 int           帧信息长度
 	//       int           帧长度
	typedef Infra::TFunction3<void,CMediaPacket *,int, int > Proc;
	//传输少量的非流媒体数据.
	//参数： data: 数据缓冲区。len: 数据的长度。
	void SendOtherData( const char* data, int len );
	//获取当前已经发了一半，但还未发送完整的包的残留部分。
	//参数：buf和max_len是用于接收该残留部分的的缓冲区及其大小。
	//返回值：大于等于0表示残留包的大小，小于0表示失败。
	//注意：调用该接口后，CMediaStreamSender认为该残留包已经发送出去，下次不会再发送。
	int GetTailPacket( const char* buf, int max_len );
	//创建一个CMediaStreamSender对象，
	//返回值： 成功则返回一个CMediaStreamSender对象，否则返回NULL。
	static CMediaStreamSender * Create();
	//调用者通过这个接口，告诉CMediaStreamSender，如果socket发生异常，就需要
	//向调用者发送消息MSG_SOCK_EXCEPTION。
	//参数： receiver_id: 消息的接收者，通常就是调用者。
	void WaitException( long receiver_id );
	//绑定一个tcp socket对象，数据将在这个socket上发送。绑定之后，
	//就不可以在其它地方再对这个socket进行数据发送操作，但读操作和其它操作可正常进行。
	//参数： stream: 待绑定的stream对象。
	//返回值：0成功，-1失败。
	int Attach( CSock * stream );
	//解绑定一个tcp socket对象，解绑定之后，CStreamSender失去发送数据的能力。
	//返回值：被解绑定的stream对象。
	CSock * Detach();
	//绑定一个回调函数,来处理发送成功的帧的信息
	//允许重复绑定，最后一次绑定的函数为处理函数
	//如果是普通函数，则传入&func
	//参数:Proc      typedef Infra::TFunction3<void,CMediaPacket *,int, int > Proc
	//返回值: true 成功，false 失败
	//注意:如果绑定的为成员函数，则使用方法如下
	//void classname::func(CMediaPacket*,int,int);
	//Proc proc(&classname::func,&object);
	//AttachSendProc(proc);
	bool AttachSendProc(Proc proc);
	//解绑定一个proc函数
	//返回值:  true
	bool DetachSendProc();
	//设置允许的因网络拥塞或者其他原因导致无法发送字节流的最长时间，若大于此时间，以网络异常体现
	//参数：sec：允许的超时时间，单位为秒
	void SetSendTimeout( unsigned int sec );
	//设置发送的通道号，默认情况下发送所有通道的数据
	//可以调用此接口进行多次设置，以同时支持发送多通道数据，设置-1为清空至默认
	//参数：channel：通道号，只支持0-7
	void SetSendChannel( const int channel );
	//不使能发送通道数据
	void DisableSendChannel( const int channel );
private:
	struct Internal;
	struct Internal*	m_internal;
};

//流媒体数据二级缓冲区的接口定义。用户通过继承这个类实现自己的流媒体缓冲区，
//并把这个缓冲区加入到CMediaBuffer中，Put接口就会被调用，就会得到需要的流
//媒体数据。
//注意：用户实现的类必须通过new创建出来，再加入到CMediaBuffer中。
class NETFRAMEWORK_API CLevel2Buffer_IF
{
public:
	CLevel2Buffer_IF(): m_next(NULL){}
	virtual ~CLevel2Buffer_IF(){}
public:
	//向二级缓冲区输入一个流媒体数据包。这个接口由CMediaBuffer调用，用户需自己实现。
	//参数： packet: 待输入的数据包。
	//			 len: 数据包的长度。
	//			 mark: 标记位，若当前包是帧的最后一包，mark==1，否则, mark==0。
	//			 level: 帧的重要等级
	//返回值： 0表示接受数据包，-1表示接受失败，CMediaBuffer将直接通过delete删除该缓冲区。
	virtual int Put( CMediaPacket* packet, int len, int mark, int level ) = 0;
private:
	friend class CMediaBuffer;
	CLevel2Buffer_IF* m_next;
};

//流媒体数据缓冲区。
class NETFRAMEWORK_API CMediaBuffer{
	CMediaBuffer& operator=( CMediaBuffer const & other );
	CMediaBuffer( CMediaBuffer const & other );
public:
	CMediaBuffer();
	virtual ~CMediaBuffer();
public:
	//添加一个流媒体发送者,加入之后，该发送者就从缓冲区最实时的位置开始
	//发送流媒体数据。
	//参数： Sender: 加入的流媒体发送者。
	void AddSender( CMediaStreamSender * Sender );
	//从缓冲区中删除一个流媒体发送者。
	//参数： Sender: 要删除的流媒体发送者。
	void DelSender( CMediaStreamSender * Sender );
	//添加一个流媒体二级缓冲区,
	//参数： buffer: 加入的流媒体二级缓冲区。
	void AddLevel2Buffer( CLevel2Buffer_IF * buffer );
	//从缓冲区中删除一个流媒体二级缓冲区。
	//参数： Sender: 要删除的流媒体二级缓冲区。
	void DelLevel2Buffer( CLevel2Buffer_IF * buffer );
	//向缓冲区输入一个流媒体数据包。
	//参数： packet: 待输入的数据包。
	//			 len: 数据包的长度。
	//			 mark: 标记位，若当前包是帧的最后一包，mark==1，否则, mark==0。
	//			 level: 帧的重要等级，范围从LEVEL_HIGH到LEVEL_LOW，使用者一定要
	//							仔细设计这个等级，并在LEVEL_HIGH到LEVEL_LOW的范围内大致均匀分布，
	//							它会影响到网络发送受阻时的丢帧，恢复的质量。
	//                          高8bits可以携带通道信息（0-7），
	//                          以利用CMediaStreamSender的SetSendChannel接口进行通道数据过滤
	//返回值： 0。
	int Put( CMediaPacket* packet, int len, int mark, int level );
	//向缓冲区输入一个帧信息包
	//参数:frameinfo:等待输入的帧信息包
	//		:len:数据包的长度
	//返回值:0.
	int PutFrameInfo(CMediaPacket * frameinfo,int len);
	//清空之前Put入MediaBuffer但还未发送出去的数据
	void Clear();
	//使能自动发送机制，则框架会以固定周期发送数据包，适用于帧率较低，但码流较大时使用
	void EnableAutoSend();
	//使能自动发送机制，框架以指定周期发送数据包，适用于帧率较低，但码流较大时使用
	//参数:period:自动发送周期，单位: 毫秒
	void EnableAutoSend(uint64_t period);
	//关闭自动发送机制
	void DisableAutoSend();
	//获取缓冲的buffer个数
	int GetBufferNum();
	// 获取缓冲的帧字节数
	uint64_t GetBufferSize();
private:
	struct Internal;
	struct Internal*		m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_MEDIA_STREAM_SENDER_H__
