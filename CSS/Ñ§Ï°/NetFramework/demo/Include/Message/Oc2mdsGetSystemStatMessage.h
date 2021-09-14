//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_OC2MDS_GET_SYSTEM_STAT_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_OC2MDS_GET_SYSTEM_STAT_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"

namespace Dahua {
namespace EFS {

//类名 	   COc2mdsGetSystemStatMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   OC2MDS_GET_SYSTEM_STAT_MESSAGE
//发起方   客户端
//接收方   SDK
//描述	   客户端向mds获取系统信息

//类名 	   CResOc2mdsGetSystemStatMessage
//协议类型 DS_PROTOCOL_RES
//协议名   OC2MDS_GET_SYSTEM_STAT_MESSAGE
//发起方   SDK
//接收方   客户端
//描述	   正常,mds答复SDK系统信息;出错,响应错误信息

class COc2mdsGetSystemStatMessage : public CPacketBase
{
public:
	COc2mdsGetSystemStatMessage( );
	COc2mdsGetSystemStatMessage( uint32_t seq );
	~COc2mdsGetSystemStatMessage( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//协议内容
private:
	class Internel;
	Internel *m_internel;
};

class CResOc2mdsGetSystemStatMessage : public CPacketBase
{
public:
	CResOc2mdsGetSystemStatMessage( );
	CResOc2mdsGetSystemStatMessage( uint32_t seq );
	~CResOc2mdsGetSystemStatMessage( );

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
	
	uint32_t getCurWriteFiles() const;
	void setCurWriteFiles(uint32_t curWriteFiles);
	uint64_t getSpaceLeft() const;
	void setSpaceLeft(uint64_t spaceLeft);
	uint32_t getTotalClients() const;
	void setTotalClients(uint32_t totalClients);
	uint64_t getTotalFiles() const;
	void setTotalFiles(uint64_t totalFiles);
	uint32_t getTotalNodes() const;
	void setTotalNodes(uint32_t totalNodes);
	uint64_t getTotalSpace() const;
	void setTotalSpace(uint64_t totalSpace);
	std::string getMdsUuid() const;
	void setMdsUuid(std::string uuid);
private:
	class Internel; 
	Internel *m_internel;
};	

typedef Memory::TSmartObjectPtr<COc2mdsGetSystemStatMessage> TOc2mdsGetSystemStatMsgPtr;
typedef Memory::TSmartObjectPtr<CResOc2mdsGetSystemStatMessage> TResOc2mdsGetSystemStatMsgPtr;

TPacketBasePtr newCOc2mdsGetSystemStatMessage(  );
TPacketBasePtr newCResOc2mdsGetSystemStatMessage(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_OC2MDS_GET_SYSTEM_STAT_MESSAGE_H__
