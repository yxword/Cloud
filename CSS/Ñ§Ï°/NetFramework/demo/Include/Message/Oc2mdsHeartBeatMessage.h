//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_OC2MDS_HEARTBEAT_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_OC2MDS_HEARTBEAT_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {
//类名 	   COc2mdsHeartBeatMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   OC2MDS_HEARTBEAT_MESSAGE
//发起方   客户端
//接收方   MDS
//描述	   客户端向MDS发送心跳

//类名 	   CResOc2mdsHeartBeatMessage
//协议类型 DS_PROTOCOL_RES
//协议名   OC2MDS_HEARTBEAT_MESSAGE
//发起方   MDS
//接收方   客户端
//描述	   MDS响应ACK


class COc2mdsHeartBeatMessage : public CPacketBase
{
public:
	COc2mdsHeartBeatMessage( );
	COc2mdsHeartBeatMessage( uint32_t seq );
	~COc2mdsHeartBeatMessage( );

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


class CResOc2mdsHeartBeatMessage : public CPacketBase
{
public:
	CResOc2mdsHeartBeatMessage( );
	CResOc2mdsHeartBeatMessage( uint32_t seq );
	~CResOc2mdsHeartBeatMessage( );

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

typedef Memory::TSmartObjectPtr<COc2mdsHeartBeatMessage> TOc2mdsHeartBeatMsgPtr;
typedef Memory::TSmartObjectPtr<CResOc2mdsHeartBeatMessage> TResOc2mdsHeartBeatMsgPtr;

TPacketBasePtr newCOc2mdsHeartBeatMessage(  );
TPacketBasePtr newCResOc2mdsHeartBeatMessage(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_OC2MDS_HEARTBEAT_MESSAGE_H__
