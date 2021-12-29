//
//  "$Id: DhtsAcceptor.h 7672 2010-5-10 02:28:18Z rao_ping $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2017-12-10		rao_ping	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_DHTS_ACCEPTOR_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_DHTS_ACCEPTOR_H__

#include "NDefs.h"
#include "DhtsStream.h"
#include "SockAddr.h"
#include <string.h>

namespace Dahua{
namespace NetFramework{

//这是一个TCP侦听套接字。
class NETFRAMEWORK_API CDhtsAcceptor : public CSock 
{
	CDhtsAcceptor( CDhtsAcceptor const & other );
	CDhtsAcceptor& operator=( CDhtsAcceptor const & other );
public:
	CDhtsAcceptor();
	~CDhtsAcceptor();
public:
	//打开一个侦听队列。
	//参数：local用于侦听的本地地址。
	//返回值：-1失败，0成功
	int Open( const CSockAddr & local );
	//接受一个远端连接请求
	//参数：remote发起请求的远端地址。
	//返回值： NULL接收失败 指针有效即为接收成功后的一个地址套接字，指针由调用者管理，负责释放。
	CDhtsStream* Accept( CSockAddr * remote = NULL );
	//接受一个远端的连接请求
	//参数：sock用于填充接受的连接请求
	//		remote远端地址
	//返回值：-1：接收失败，sock不可用 0：成功，sock可用
	int Accept( CDhtsStream& sock, CSockAddr * remote = NULL );
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_DHTS_ACCEPTOR_H__

