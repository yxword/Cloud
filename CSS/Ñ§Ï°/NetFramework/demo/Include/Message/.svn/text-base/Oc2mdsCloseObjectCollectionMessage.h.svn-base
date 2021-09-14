//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_OC2MDS_CLOSE_OBJECT_COLLECTION_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_OC2MDS_CLOSE_OBJECT_COLLECTION_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {
//类名 	   COc2mdsCloseObjectCollectionMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   CLOSE_FILE_MESSAGE
//发起方   客户端
//接收方   MDS
//描述	   客户端向MDS申请创建文件

//类名 	   CResOc2mdsCloseObjectCollectionMessage
//协议类型 DS_PROTOCOL_RES
//协议名   CLOSE_FILE_MESSAGE
//发起方   MDS
//接收方   客户端
//描述	   返回关闭状态


class COc2mdsCloseObjectCollectionMessage : public CPacketBase
{
public:
	COc2mdsCloseObjectCollectionMessage( );
	COc2mdsCloseObjectCollectionMessage( uint32_t seq );
	~COc2mdsCloseObjectCollectionMessage( );

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

	//设置/获取文件id
	void setFileId( uint64_t fileId );
	uint64_t getFileId( ) const;

	//设置/获取最后一个object的id,用于写操作时,告诉mds最后使用的是哪个object
	void setLastObjectId( uint16_t lastObjectId );
	uint16_t getLastObjectId( ) const;

	//设置之前打开文件进行操作是读还是写,1读2写
	void setFileMode( uint16_t mode );
	uint16_t getFileMode() const;

	//设置和获取文件长度
	void setFileLength( int64_t length );
	int64_t  getFileLength( ) const;
private:
	class Internel;
	Internel *m_internel;
};


class CResOc2mdsCloseObjectCollectionMessage : public CPacketBase
{
public:
	CResOc2mdsCloseObjectCollectionMessage( );
	CResOc2mdsCloseObjectCollectionMessage( uint32_t seq );
	~CResOc2mdsCloseObjectCollectionMessage( );

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

typedef Memory::TSmartObjectPtr<COc2mdsCloseObjectCollectionMessage> TOc2mdsCloseObjectCollectionMsgPtr;
typedef Memory::TSmartObjectPtr<CResOc2mdsCloseObjectCollectionMessage> TResOc2mdsCloseObjectCollectionMsgPtr;

TPacketBasePtr newCOc2mdsCloseObjectCollectionMessage(  );
TPacketBasePtr newCResOc2mdsCloseObjectCollectionMessage(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_OC2MDS_CLOSE_OBJECT_COLLECTION_MESSAGE_H__
