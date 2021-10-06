//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_CREATE_OBJECT_COLLECTION_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_CREATE_OBJECT_COLLECTION_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {
//类名 	   COc2mdsCreateObjectCollectionMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   OC2MDS_CREATE_OBJECT_COLLECTION_MESSAGE
//发起方   客户端
//接收方   MDS
//描述	   客户端向MDS申请创建文件

//类名 	   CResOc2mdsCreateObjectCollectionMessage
//协议类型 DS_PROTOCOL_RES
//协议名   OC2MDS_CREATE_OBJECT_COLLECTION_MESSAGE
//发起方   MDS
//接收方   客户端
//描述	   正常,MDS向客户端响应大文件的文件id;出错,响应错误信息


class COc2mdsCreateObjectCollectionMessage : public CPacketBase
{
public:
	COc2mdsCreateObjectCollectionMessage( );
	COc2mdsCreateObjectCollectionMessage( uint32_t seq );
	~COc2mdsCreateObjectCollectionMessage( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//协议内容,大文件1，小文件2
    void setFileType( uint8_t type );
    uint8_t getFileType() const;

    void setBucketId( uint64_t id );
    uint64_t getBucketId() const;
    
	//冗余类型
	void setRule( int16_t type );
	int16_t getRule( ) const;

private:
	class Internel;
	Internel *m_internel;
};


class CResOc2mdsCreateObjectCollectionMessage : public CPacketBase
{
public:
	CResOc2mdsCreateObjectCollectionMessage( );
	CResOc2mdsCreateObjectCollectionMessage( uint32_t seq );
	~CResOc2mdsCreateObjectCollectionMessage( );

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
	//设置file id所在的簇索引
	void setFamilyIndex( uint8_t index );
	//获取file id所在的簇索引
	uint8_t getFamilyIndex( )const;
	//设置file id 所在的集群号
	void setClusterId( uint8_t cid );
	//获取file id 所在的集群号
	uint8_t getClusterId( )const;
	//设置文件类型
	void setFileType( uint8_t type );
	//获取文件类型，大文件1，小文件2
	uint8_t getFileType( )const;
	//设置file id
	void setFileId( uint64_t fileId );
	//获取file id
	uint64_t getFileId( )const;

    // 设置真实数据块数。
    void setDataNum(int8_t dataNum);
    // 获取真实数据块数。
    int8_t getDataNum() const;

    // 设置冗余数据块数。
    void setParityNum(int8_t parityNum);
    // 获取冗余数据块数。
    int8_t getParityNum() const;

    // EC算法，0表示副本，1表示XOR。
    void setECType(int8_t ECType);
    // 获取EC算法，0表示副本，1表示XOR。
    int8_t getECType() const;

    // 设置分片大小。
    void setPieceSize(int32_t pieceSize);
    // 获取分片大小。
    int32_t getPieceSize() const;

    // 设置默认object的大小。
    void setObjSize( uint32_t objSize );
    // 获取默认object的大小。
    uint32_t getObjSize() const;
private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<COc2mdsCreateObjectCollectionMessage> TOc2mdsCreateObjectCollectionMsgPtr;
typedef Memory::TSmartObjectPtr<CResOc2mdsCreateObjectCollectionMessage> TResOc2mdsCreateObjectCollectionMsgPtr;

TPacketBasePtr newCOc2mdsCreateObjectCollectionMessage(  );
TPacketBasePtr newCResOc2mdsCreateObjectCollectionMessage(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_CLOUD_STORAGE_CREATEFILE_MESSAGE_H__
