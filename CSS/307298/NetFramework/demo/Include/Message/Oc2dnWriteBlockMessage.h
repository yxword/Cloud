//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_OC2DN_WRITE_BLOCK_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_OC2DN_WRITE_BLOCK_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"

namespace Dahua {
namespace EFS {

//类名 	   COc2dnWriteBlockMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   OC2DN_WRITE_BLOCK_MESSAGE
//发起方   客户端
//接收方   DATANODE
//描述	   客户端向DATANODE写数据

//类名 	   CResOc2dnWriteBlockMessage
//协议类型 DS_PROTOCOL_RES
//协议名   OC2DN_WRITE_BLOCK_MESSAGE
//发起方   DATANODE
//接收方   客户端
//描述	   正常,DATANODE向客户端响应写入状态;出错,响应错误信息

class COc2dnWriteBlockMessage : public CPacketBase
{
public:
	COc2dnWriteBlockMessage( );
	COc2dnWriteBlockMessage( uint32_t seq );
	~COc2dnWriteBlockMessage( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//协议内容
	//设置获取块标识
	void setBlockIndex( struct BlockIndex& blkIndex );
	void getBlockIndex( struct BlockIndex& blkIndex ) const;

	//设置缓冲区
	void setBuffer(const char* buffer,int32_t length,uint32_t offsetInBlock,bool isLastInBlock=false);
	//获取缓冲区
	//WARN:如果setBuffer后,没有调用生成序列化数据,那么可能得到的buffer会是一个失效的野指针
	const char* getBuffer() const;
	//获取缓冲区长度
	int32_t getBufferLen() const;
	//获取在block中的偏移
	uint32_t getOffset() const;
	//是否是最后一个包，是则刷冲之后关闭文件
	bool isLastInBlock() const;

	//设置附加可选的key/value属性
	void setOptionalAttr(const char* key,int32_t keyLen,const char* val,int valLen);
	//获取key长度
	int32_t getAttrKeyLen() const;
	//获取key
	const char* getAttrKey() const;
	//获取value长度
	int32_t getAttrValueLen() const;
	//获取value
	const char* getAttrValue() const;
private:
	class Internel;
	Internel *m_internel;
};

class CResOc2dnWriteBlockMessage : public CPacketBase
{
public:
	CResOc2dnWriteBlockMessage( );
	CResOc2dnWriteBlockMessage( uint32_t seq );
	~CResOc2dnWriteBlockMessage( );

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
	//设置和获取是否是文件尾
	void setEOF(bool isEndOfFile);
	bool isEOF() const;

private:
	class Internel; 
	Internel *m_internel;
};	

typedef Memory::TSmartObjectPtr<COc2dnWriteBlockMessage> TOc2dnWriteBlockMsgPtr;
typedef Memory::TSmartObjectPtr<CResOc2dnWriteBlockMessage> TResOc2dnWriteBlockMsgPtr;

TPacketBasePtr newCOc2dnWriteBlockMessage(  );
TPacketBasePtr newCResOc2dnWriteBlockMessage(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_OC2DN_WRITE_BLOCK_MESSAGE_H__
