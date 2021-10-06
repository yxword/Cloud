//
//  "$Id: SimulatorStream.h 30412 2011-07-21 05:57:44Z qin_fenglin $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-7-6		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SIMULATOR_STREAM_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SIMULATOR_STREAM_H__

#include "SockStream.h"
#include "NDefs.h"
#ifdef WIN32
#include "NTypes_Win32.h"
#else
#include <sys/uio.h>
#endif

namespace Dahua{
namespace NetFramework{

//网络流量仿真套接字。
//这是一个用于测试用途的套接字，通过SetBandWidth设置要仿真的带宽。
//注意：这个socket用于测试用途，即使不限流量，它的效率也要远低于其它的socket，
//所以，在正式代码中不要使用这个socket。
class NETFRAMEWORK_API CSimulatorStream : public CSock
{
	CSimulatorStream( CSimulatorStream const & other );
	CSimulatorStream& operator=( CSimulatorStream const & other );
public:
	CSimulatorStream();
	~CSimulatorStream();
	//设置真实的socket，所有的数据发送操作，实际上是在这个socket上发生的。
	//设置后，调用CSimulatorStream的发送接口，即获取流量限制的功能，对真实socket上所有
	//原有的功能不产生任何影响。如果不设置这个接口，也不影响使用，只是没有数据被实际
	//发送出。
	//参数： stream: 真实的tcp socket.
	void SetRealSocket( CSockStream *stream );
	//设置仿真网络带宽，设置完毕后，该套接字上的发送速度不能超出这个带宽值。
	//参数： mbps： 要设置的带宽值，单位为Mb/s。
	//返回值： 0。
	int SetBandWidth( uint32_t mbps );
	//发送TCP数据
	//参数：buf待发送数据 len待发送数据长度。
	//返回值：-1失败，0未发送下次再试，大于0表示发送出去的长度。
	int Send( const char * buf, uint32_t len );
	//同时发送存在于多个缓冲区的TCP数据。
	//参数： vector：同linux系统调用writev的参数。
	//				count: vector中缓冲区的数量。
	//返回值：-1失败，0未发送下次再试，大于0表示发送出去的长度。 
	int WriteV( const struct iovec *vector, int count );
private:
	struct Internal;	
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SIMULATOR_STREAM_H__
