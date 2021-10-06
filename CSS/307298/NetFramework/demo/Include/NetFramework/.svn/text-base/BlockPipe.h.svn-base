//
//  "$Id: BlockPipe.h 30412 2011-07-21 05:57:44Z qin_fenglin $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-7-4		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_BLOCK_PIPE_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_BLOCK_PIPE_H__

#include "Infra/IntTypes.h"
#include "NetHandler.h"
#include "NDefs.h"

namespace Dahua{
namespace NetFramework{
	
//阻塞管道。
//该对象的创建必须通过静态成员方法Create进行。
class NETFRAMEWORK_API CBlockPipe : public CNetHandler
{
	CBlockPipe& operator=( CBlockPipe const & other );
	CBlockPipe( CBlockPipe const & other );
public:
	//创建一个管道，timeout为管道的超时时间，单位为微秒（百万分之一秒）
	//返回值： NULL失败，成功则返回一个创建完成的阻塞管道对象。
	static CBlockPipe * Create( int64_t timeout );
	//关闭管道，new操作创建出来的CBlockPipe最后调用这个接口即成功关闭释放，
	//无需delete。
	//返回值： 0成功，-1失败。
	void Close();
	//从管道读取数据，这是一个阻塞接口，直到有数据可读或超时才返回。
	//参数： buf: 存放读到的数据的缓冲区。
	//			 len: 缓冲区的长度。
	//返回值： 大于0表示读到的数据的长度，等于0表示超时时间已到，-1表示出错。
	int Read( char * buf, uint32_t len );
	//向管道写数据，这是一个阻塞接口。
	//参数：buf: 待写数据。
	//			len: 待写数据的长度。
	//返回值： 本次写入成功的数据量，-1表示出错。
	int Write( const char * buf, uint32_t len );
private:
	struct Internal;
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_BLOCK_PIPE_H__
