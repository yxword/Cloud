//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_OC2DN_PREPARE_BLOCK_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_OC2DN_PREPARE_BLOCK_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"

namespace Dahua {
namespace EFS {
//类名 	   COc2dnPrepareBlockMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   OC2DN_PREPARE_BLOCK_MESSAGE
//发起方   客户端
//接收方   DATANODE
//描述	   客户端向DATANODE创建block，发送block相关元数据信息

//类名 	   CResOc2dnPrepareBlockMessage
//协议类型 DS_PROTOCOL_RES
//协议名   OC2DN_PREPARE_BLOCK_MESSAGE
//发起方   DATANODE
//接收方   客户端
//描述	   正常,DATANODE向客户端响应OK;出错,响应错误信息

class COc2dnPrepareBlockMessage : public CPacketBase
{
public:
	typedef enum{ 
		writeData = 0,    // 从空状态写入数据
		recoverData,      // dn间恢复数据
		appendData        // 在已有block上append数据
	}PushMode;

public:
	COc2dnPrepareBlockMessage( );
	COc2dnPrepareBlockMessage( uint32_t seq );
	~COc2dnPrepareBlockMessage( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//协议内容
	//设置获取DataNode id
	void setDataNodeId( const std::string& id );
	std::string getDataNodeId() const;

	//设置获取BlockIndex
	void setBlockIndex( const struct BlockIndex& blkIndex );
	void getBlockIndex( struct BlockIndex& blkIndex ) const;

	//设置获取数据冗余规则
	void setRuleInfo( uint8_t dataNum, uint8_t parityNum, uint8_t ECType, uint32_t pieceSize );
	void getRuleInfo( uint8_t& dataNum, uint8_t& parityNum, uint8_t& ECType, uint32_t& pieceSize ) const;

	//设置获取block所属的object在文件中的偏移
	void setObjectOffset( uint64_t offset );
	uint64_t getObjectOffset() const;

	//设置数据写入的模式,需要将PushMode转换为uint8_t
    void setPushDataMode(uint8_t pushMode);
    uint8_t getPushDataMode() const;

    //设置获取mds 分配的block的大小
    void setBlockSize(uint64_t blockSize);
    uint64_t getBlockSize() const;

    //设置获取当前的block文件的的偏移
    void setBlockOffset( uint64_t offset );
    uint64_t getBlockOffset() const;
private:
	class Internel;
	Internel *m_internel;
};

class CResOc2dnPrepareBlockMessage : public CPacketBase
{
public:
	CResOc2dnPrepareBlockMessage( );
	CResOc2dnPrepareBlockMessage( uint32_t seq );
	~CResOc2dnPrepareBlockMessage( );

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
private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<COc2dnPrepareBlockMessage> TOc2dnPrepareBlockMsgPtr;
typedef Memory::TSmartObjectPtr<CResOc2dnPrepareBlockMessage> TResOc2dnPrepareBlockMsgPtr;

TPacketBasePtr newCOc2dnPrepareBlockMessage(  );
TPacketBasePtr newCResOc2dnPrepareBlockMessage(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_OC2DN_PREPARE_BLOCK_MESSAGE_H__
