//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_OC2DN_READ_BLOCK_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_OC2DN_READ_BLOCK_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"

namespace Dahua {
namespace EFS {
//类名 	   COc2dnReadBlockMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   OC2DN_READ_BLOCK_MESSAGE
//发起方   客户端
//接收方   DATANODE
//描述	   客户端从DATANODE读数据块中的数据

//类名 	   CResOc2dnReadBlockMessage
//协议类型 DS_PROTOCOL_RES
//协议名   OC2DN_READ_BLOCK_MESSAGE
//发起方   DATANODE
//接收方   客户端
//描述	   正常,DATANODE向客户端返回数据;出错,响应错误信息

class COc2dnReadBlockMessage : public CPacketBase
{
public:
	COc2dnReadBlockMessage( );
	COc2dnReadBlockMessage( uint32_t seq );
	~COc2dnReadBlockMessage( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//协议内容
	//设置和获取block标识
	void setBlockIndex( struct BlockIndex& blkIndex );
	void getBlockIndex( struct BlockIndex& blkIndex ) const;

	//设置和获取需要读的长度
	void setReadLen(int32_t len);
	int32_t getReadLen() const;

	//设置和获取起始位置
	void setStartOffset(uint32_t offset);
	uint32_t getStartOffset() const;

private:
	class Internel;
	Internel *m_internel;
};


class CResOc2dnReadBlockMessage : public CPacketBase
{
public:
	CResOc2dnReadBlockMessage( );
	CResOc2dnReadBlockMessage( uint32_t seq );
	~CResOc2dnReadBlockMessage( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//协议内容
	//包有错误,则设置错误类型进去,否则默认包是正确的
	void setErrorType( int32_t errorType );
	//获取包是否有错,0表示正确,-1标识错误
	int8_t isError( ) const;
	//如果包出错,则获取到的错误类型标识是什么错误
	int32_t getErrorType( ) const;
	
	//设置缓冲区
	void setBuffer(const char* buffer,int32_t len,bool isEOF = false);
	//获取缓冲区的信息
	//WARN:如果setBuffer后,没有调用生成序列化数据,那么可能得到的buffer会是一个失效的野指针
	const char* getBuffer() const;
	int32_t getBufferLen() const;
	bool isEOF() const;

private:
	class Internel; 
	Internel *m_internel;
};	

typedef Memory::TSmartObjectPtr<COc2dnReadBlockMessage> TOc2dnReadBlockMsgPtr;
typedef Memory::TSmartObjectPtr<CResOc2dnReadBlockMessage> TResOc2dnReadBlockMsgPtr;

TPacketBasePtr newCOc2dnReadBlockMessage(  );
TPacketBasePtr newCResOc2dnReadBlockMessage(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_OC2DN_READ_BLOCK_MESSAGE_H__
