//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_CLOUD_STORAGE_DN2MDS_HEARTBEAT_MESSAGE_H__
#define __INCLUDE_DAHUA_CLOUD_STORAGE_DN2MDS_HEARTBEAT_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"

namespace Dahua {
namespace EFS {
//类名 	   CDn2mdsHeartBeatMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   DN2MDS_HEARTBEAT_MESSAGE
//发起方   DATANODE
//接收方   MDS
//描述	   DATANODE向MDS发送心跳包

class CDn2mdsHeartBeatMessage : public CPacketBase
{
public:
	CDn2mdsHeartBeatMessage( );
	CDn2mdsHeartBeatMessage( uint32_t seq );
	~CDn2mdsHeartBeatMessage( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//协议内容
	//设置和获取系统容量信息
	void setCapacityInfo( int64_t capacity,int64_t dfsUsed,int64_t remaining );
	void getCapacityInfo( int64_t& capacity,int64_t& dfsUsed,int64_t& remaining ) const;

	//设置和获取网络信息
	void setNetInfo( int32_t connectionCount,int64_t inBandWidth,int64_t outBandWidth );
	void getNetInfo( int32_t& connectionCount,int64_t& inBandWidth,int64_t& outBandWidth ) const;

	//设置和获取系统cpu信息
	//totalCpuMhz cpu总赫兹，单位为MHz
	//loadPercent 负载百分比,精确到小数点后两位
	void setCpuInfo( uint32_t totalCpuMhz,double loadPercent );
	void getCpuInfo( uint32_t& totalCpuMhz,double& loadPercent ) const;

	//设置和获取磁盘io负载
	//loadPercent 负载百分比,精确到小数点后两位
	void setIOLoad( double loadPercent );
	double getIOLoad( ) const;

	//设置和获取网络总吞吐能力
	//maxNetBandWidth 最大网络带宽,单位为字节
	void setNetLimit( int64_t maxNetBandWidth );
	int64_t getNetLimit() const;
private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CDn2mdsHeartBeatMessage> TDn2mdsHeartBeatMsgPtr;

TPacketBasePtr newCDn2mdsHeartBeatMessage(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_CLOUD_STORAGE_DN2MDS_HEARTBEAT_MESSAGE_H__
