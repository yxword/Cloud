//
//  "$Id: HeartBeatMessage.h  2014年10月8日  zhang_hailong 23919 $"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day     SVN-Author         Modification
//					2014年10月8日	   zhang_hailong	  Create
//

#ifndef __INCLUDE_DAHUA_EFS_HEARTBEATMESSAGE_H__
#define __INCLUDE_DAHUA_EFS_HEARTBEATMESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"
#include "Common/InnerVersion.h"

//#include <vector>

namespace Dahua {
namespace EFS {

class CHeartBeatMessage: public CPacketBase
{
public:
	enum HBServiceType
	{
		hbUnknownService = -1,
		hbMdsService = 0,
		hbCsService = 1
	};

	CHeartBeatMessage( HBServiceType type, bool request, uint32_t seq = 0 );
	virtual ~CHeartBeatMessage();

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData();
	//获取序列化的数据的长度
	uint32_t getDataLen();
	//打印包用于debug
	void printPacket() const;

	// mds uid, MDS 心跳使用
	void setMdsUid( const std::string& uid );
	std::string getMdsUid() const;

	// cluster id, MDS 心跳使用
	void setClusterId( uint8_t id );
	uint8_t getClusterId() const;

	// mds IP, CS 心跳使用
	void setMdsIp( const std::string& ip );
	std::string getMdsIp() const;

	// 虚IP, MDS CS 通用
	void setVirtualIp( const std::string& ip );
	std::string getVirtualIp() const;

	// 本机IP, MDS CS 通用
	void setLocalIp( const std::string& ip );
	std::string getLocalIp() const;

	// 对端IP, MDS CS 通用
	void setPeerIp( const std::string& ip );
	std::string getPeerIp() const;

	// 本机mac, MDS 心跳保留字段
	void setMacAddr( const std::string& mac );
	std::string getMacAddr() const;

	// MDS 心跳保留字段
	void setIpCount( const uint8_t count );
	uint8_t getIpCount() const;

	// MDS 心跳保留字段
	void setVersion( const CInnerVersion version );
	CInnerVersion getVersion() const;

	// 服务类型,区分是MDS心跳还是CS心跳
	void setServiceType( const int8_t type );
	int8_t getServiceType() const;

	// 本机历史状态
	void setUsedState( const int8_t state );
	int8_t getUsedState() const;

	// 本机当前状态
	void setCurrentState( const int8_t state );
	int8_t getCurrentState() const;

	// 本机成为master的时间戳
	void setMasterTimeStamp( const int32_t timeStamp );
	int32_t getMasterTimeStamp() const;
private:
	class Internel;
	Internel *m_internel;

};

typedef Memory::TSmartObjectPtr<CHeartBeatMessage> THeatBeatMsgPtr;

TPacketBasePtr newCSHeartbeatMessageRequest();
TPacketBasePtr newCSHeartbeatMessageResponse();

TPacketBasePtr newMdsHeartbeatMessageRequest();
TPacketBasePtr newMdsHeartbeatMessageResponse();
} /* namespace EFS */
} /* namespace Dahua */
#endif /* __DAHUA_EFS_HEARTBEATMESSAGE_H__ */
