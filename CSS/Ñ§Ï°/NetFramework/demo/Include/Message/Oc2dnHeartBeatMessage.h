//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_OC2DN_HEARTBEAT_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_OC2DN_HEARTBEAT_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {
//类名 	   COc2dnHeartBeatMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   OC2DN_HEARTBEAT_MESSAGE
//发起方   客户端
//接收方   DATANODE
//描述	   客户端向DATANODE发送心跳

//类名 	   CResOc2dnHeartBeatMessage
//协议类型 DS_PROTOCOL_RES
//协议名   OC2DN_HEARTBEAT_MESSAGE
//发起方   DATANODE
//接收方   客户端
//描述	   DATANODE响应ACK


class COc2dnHeartBeatMessage : public CPacketBase
{
public:
	COc2dnHeartBeatMessage( );
	COc2dnHeartBeatMessage( uint32_t seq );
	~COc2dnHeartBeatMessage( );

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


class CResOc2dnHeartBeatMessage : public CPacketBase
{
public:
	CResOc2dnHeartBeatMessage( );
	CResOc2dnHeartBeatMessage( uint32_t seq );
	~CResOc2dnHeartBeatMessage( );

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

typedef Memory::TSmartObjectPtr<COc2dnHeartBeatMessage> TOc2dnHeartBeatMsgPtr;
typedef Memory::TSmartObjectPtr<CResOc2dnHeartBeatMessage> TResOc2dnHeartBeatMsgPtr;

TPacketBasePtr newCOc2dnHeartBeatMessage(  );
TPacketBasePtr newCResOc2dnHeartBeatMessage(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_OC2DN_HEARTBEAT_MESSAGE_H__
