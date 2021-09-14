//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_OC2MDS_ALLOC_STORAGE_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_OC2MDS_ALLOC_STORAGE_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"
namespace Dahua {
namespace EFS {
//类名 	   COc2mdsAllocStorageMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   OC2MDS_ALLOC_STORAGE_MESSAGE
//发起方   客户端
//接收方   MDS
//描述	   客户端向MDS申请创建文件

//类名 	   CResOc2mdsAllocStorageMessage
//协议类型 DS_PROTOCOL_RES
//协议名   OC2MDS_ALLOC_STORAGE_MESSAGE
//发起方   MDS
//接收方   客户端
//描述	   正常,MDS向客户端响应大文件的文件id;出错,响应错误信息


class COc2mdsAllocStorageMessage : public CPacketBase
{
public:
	COc2mdsAllocStorageMessage( );
	COc2mdsAllocStorageMessage( uint32_t seq );
	~COc2mdsAllocStorageMessage( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//协议内容
	void setBucketId(uint64_t id);
	uint64_t getBucketId() const;

	void setFileId( uint64_t fileId );
	uint64_t getFileId( ) const;

	void setFileOffset( uint64_t fileOffset );
	uint64_t getFileOffset( ) const;

	//设置/获取排除的数据节点
	void setExclude( std::vector<struct DataNodeId>& excludeNode );
	void getExclude( std::vector<struct DataNodeId>& excludeNode ) const;

private:
	class Internel;
	Internel *m_internel;
};


class CResOc2mdsAllocStorageMessage : public CPacketBase
{
public:
	CResOc2mdsAllocStorageMessage( );
	CResOc2mdsAllocStorageMessage( uint32_t seq );
	~CResOc2mdsAllocStorageMessage( );

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
	//设置文件最后一个object
	void setLastObject( struct LocatedObject& obj );
	//获取文件最后一个object
	void getLastObject( struct LocatedObject& obj ) const;

private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<COc2mdsAllocStorageMessage> TOc2mdsAllocStorageMsgPtr;
typedef Memory::TSmartObjectPtr<CResOc2mdsAllocStorageMessage> TResOc2mdsAllocStorageMsgPtr;

TPacketBasePtr newCOc2mdsAllocStorageMessage( );
TPacketBasePtr newCResOc2mdsAllocStorageMessage( );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_STORAGELLOC_STORAGE_MESSAGE_H__
