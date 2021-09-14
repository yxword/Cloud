//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_OC2MDS_CREATE_OBJECT_BUCKET_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_OC2MDS_CREATE_OBJECT_BUCKET_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"

namespace Dahua {
namespace EFS {

//类名 	   COc2mdsCreateObjectBucketMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   OC2MDS_CREATE_OBJECT_BUCKET_MESSAGE
//发起方   客户端
//接收方   mds
//描述	   客户端向mds创建bucket

//类名 	   CResOc2mdsCreateObjectBucketMessage
//协议类型 DS_PROTOCOL_RES
//协议名   OC2MDS_CREATE_OBJECT_BUCKET_MESSAGE
//发起方   mds
//接收方   客户端
//描述	   正常,mds创建bucket成功;出错,响应错误信息

class COc2mdsCreateObjectBucketMessage : public CPacketBase
{
public:
	COc2mdsCreateObjectBucketMessage( );
	COc2mdsCreateObjectBucketMessage( uint32_t seq );
	~COc2mdsCreateObjectBucketMessage( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

private:
	class Internel;
	Internel *m_internel;
};

class CResOc2mdsCreateObjectBucketMessage : public CPacketBase
{
public:
	CResOc2mdsCreateObjectBucketMessage( );
	CResOc2mdsCreateObjectBucketMessage( uint32_t seq );
	~CResOc2mdsCreateObjectBucketMessage( );

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
	
	//设置获取bucketid
	void setBucketId(uint64_t bucketId);
	uint64_t getBucketId() const;

private:
	class Internel; 
	Internel *m_internel;
};	

typedef Memory::TSmartObjectPtr<COc2mdsCreateObjectBucketMessage> TOc2mdsCreateObjectBucketMsgPtr;
typedef Memory::TSmartObjectPtr<CResOc2mdsCreateObjectBucketMessage> TResOc2mdsCreateObjectBucketMsgPtr;

TPacketBasePtr newCOc2mdsCreateObjectBucketMessage(  );
TPacketBasePtr newCResOc2mdsCreateObjectBucketMessage(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_OC2MDS_CREATE_OBJECT_BUCKET_MESSAGE_H__
