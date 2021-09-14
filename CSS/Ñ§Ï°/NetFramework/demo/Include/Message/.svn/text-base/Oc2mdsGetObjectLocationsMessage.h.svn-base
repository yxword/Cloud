//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_OC2MDS_GET_OBJECT_LOCATIONS_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_OC2MDS_GET_OBJECT_LOCATIONS_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {
//类名 	   COc2mdsGetObjectLocationsMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   OC2MDS_GET_OBJECT_LOCATIONS_MESSAGE
//发起方   客户端
//接收方   MDS
//描述	   客户端向MDS申请创建文件

//类名 	   CResOc2mdsGetObjectLocationsMessage
//协议类型 DS_PROTOCOL_RES
//协议名   OC2MDS_GET_OBJECT_LOCATIONS_MESSAGE
//发起方   MDS
//接收方   客户端
//描述	   正常,MDS向客户端响应大文件的文件id;出错,响应错误信息

class COc2mdsGetObjectLocationsMessage : public CPacketBase
{
public:
	COc2mdsGetObjectLocationsMessage( );
	COc2mdsGetObjectLocationsMessage( uint32_t seq );
	~COc2mdsGetObjectLocationsMessage( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//协议内容
    //设置/获取bucketid
    void setBucketId(uint64_t bucketId);
    uint64_t getBucketId() const;

	//设置/获取文件id
	void setFileId(uint64_t fileId);
	uint64_t getFileId() const;

	//设置/获取读取范围
	void setRange(int64_t offset,int64_t length);
	void getRange(int64_t& offset, int64_t& length) const;

	//设置/获取是否第一次打开
	void setIsFirst(bool isFirst);
	bool isFirst() const;

private:
	class Internel;
	Internel *m_internel;
};


class CResOc2mdsGetObjectLocationsMessage : public CPacketBase
{
public:
	CResOc2mdsGetObjectLocationsMessage( );
	CResOc2mdsGetObjectLocationsMessage( uint32_t seq );
	~CResOc2mdsGetObjectLocationsMessage( );

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
	//设置一段范围内的objects
	void setLocatedObjects(struct LocatedObjects& objects);
	//获取一段范围内的objects
	void getLocatedObjects(struct LocatedObjects& objects) const;

    // 设置真实数据块数。
    void setDataNum(int8_t dataNum);
    // 获取真实数据块数。
    int8_t getDataNum() const;

    // 设置冗余数据块数。
    void setParityNum(int8_t parityNum);
    // 获取冗余数据块数。
    int8_t getParityNum() const;

    // 设置分片大小。
    void setPieceSize(int32_t pieceSize);
    // 获取分片大小。
    int32_t getPieceSize() const;

    // 设置默认object的大小。
    void setObjSize( uint32_t objSize );
    // 获取默认object的大小。
    uint32_t getObjSize() const;

    //设置/获取文件类型
    void setFileType(uint8_t type);
    uint8_t getFileType() const;
private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<COc2mdsGetObjectLocationsMessage> TOc2mdsGetObjectLocationsMsgPtr;
typedef Memory::TSmartObjectPtr<CResOc2mdsGetObjectLocationsMessage> TResOc2mdsGetObjectLocationsMsgPtr;

TPacketBasePtr newCOc2mdsGetObjectLocationsMessage(  );
TPacketBasePtr newCResOc2mdsGetObjectLocationsMessage(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_OC2MDS_GET_OBJECT_LOCATIONS_MESSAGE_H__
