//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_PROTOPARSER_H__
#define __INCLUDE_DAHUA_EFS_PROTOPARSER_H__

#include "PacketBase.h"

namespace Dahua {
namespace EFS {

//用于协议接收端，负责将字节流解析成消息
class CProtoParser
{
public:
	static bool setPacketFactory( uint8_t pktClass, uint16_t pktType, pkt_builder builder );		
public:
	CProtoParser();
	~CProtoParser();
	//获取缓存，用于输入字节流
	char* getBuffer();
	//获取缓存长度
	uint32_t getLength();
	//输入数据长度,返回完整packet数
	int putDataLen( uint32_t len );
	//获取解析结果
	TPacketBasePtr getPacket();
private:
	struct Internal;
	struct Internal*	m_internal;
};

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_PROTOPARSER_H__
