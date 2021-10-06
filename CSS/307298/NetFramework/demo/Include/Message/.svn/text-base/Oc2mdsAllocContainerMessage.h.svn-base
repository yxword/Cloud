//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_OC2MDS_ALLOC_CONTAINER_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_OC2MDS_ALLOC_CONTAINER_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {
//类名 	   COc2mdsAllocContainerMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   OC2MDS_ALLOC_CONTAINER_MESSAGE
//发起方   客户端
//接收方   MDS
//描述	   客户端向MDS申请创建container

//类名 	   CResOc2mdsAllocContainerMessage
//协议类型 DS_PROTOCOL_RES
//协议名   OC2MDS_ALLOC_CONTAINER_MESSAGE
//发起方   MDS
//接收方   客户端
//描述	   正常,MDS向客户端响应container创建后的基本信息;出错,响应错误信息

class COc2mdsAllocContainerMessage : public CPacketBase
{
public:
	COc2mdsAllocContainerMessage( );
	COc2mdsAllocContainerMessage( uint32_t seq );
	~COc2mdsAllocContainerMessage( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

    //设置获取bucket id
    void setBucketId( uint64_t id );
    uint64_t getBucketId() const;
    
	//设置获取冗余类型
	void setRule( int16_t type );
	int16_t getRule( ) const;
private:
	class Internel;
	Internel *m_internel;
};


class CResOc2mdsAllocContainerMessage : public CPacketBase
{
public:
	CResOc2mdsAllocContainerMessage( );
	CResOc2mdsAllocContainerMessage( uint32_t seq );
	~CResOc2mdsAllocContainerMessage( );

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

	// 设置获取file id 所在的集群号
	void setClusterId( uint8_t cid );
	uint8_t getClusterId( )const;

	// 设置获取file id
	void setFileId( uint64_t fileId );
	uint64_t getFileId( )const;

    // 设置获取真实数据块数。
    void setDataNum(int8_t dataNum);
    int8_t getDataNum() const;

    // 设置获取冗余数据块数。
    void setParityNum(int8_t parityNum);
    int8_t getParityNum() const;

    // 设置获取EC算法，0表示副本，1表示XOR。
    void setECType(int8_t ECType);
    int8_t getECType() const;

    // 设置获取分片大小。
    void setPieceSize(int32_t pieceSize);
    int32_t getPieceSize() const;

    // 设置获取默认object的大小。
    void setObjSize( uint32_t objSize );
    uint32_t getObjSize() const;

    // 设置获取Container已用大小
    void setContainerLength( uint64_t contaierLength );
    uint64_t getContainerLength() const;
private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<COc2mdsAllocContainerMessage> TOc2mdsAllocContainerMsgPtr;
typedef Memory::TSmartObjectPtr<CResOc2mdsAllocContainerMessage> TResOc2mdsAllocContainerMsgPtr;

TPacketBasePtr newCOc2mdsAllocContainerMessage(  );
TPacketBasePtr newCResOc2mdsAllocContainerMessage(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_CLOUD_STORAGE_CREATEFILE_MESSAGE_H__
