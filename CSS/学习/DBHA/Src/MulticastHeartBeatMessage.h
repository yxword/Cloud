

#ifndef __DAHUA_VIDEOCLOUD_DBHA_HEARTBEATMESSAGE_H__
#define __DAHUA_VIDEOCLOUD_DBHA_HEARTBEATMESSAGE_H__

#include <string>
#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"

namespace Dahua {
namespace VideoCloud {
namespace DBHA {


class CMulticastHeartBeatMessage: public EFS::CPacketBase
{
public:

    CMulticastHeartBeatMessage( bool request, uint32_t seq = 0 );
	virtual ~CMulticastHeartBeatMessage();

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData();
	//获取序列化的数据的长度
	uint32_t getDataLen();
	//打印包用于debug
	void printPacket() const;

	// UID 心跳使用
	void setUid( const std::string& uid );
	std::string getUid() const;

	// cluster id, 心跳使用
	void setClusterId( uint8_t id );
	uint8_t getClusterId() const;

	// 虚IP
	void setVirtualIp( const std::string& ip );
	std::string getVirtualIp() const;

	// 本机IP
	void setLocalIp( const std::string& ip );
	std::string getLocalIp() const;

	// 对端IP
	void setPeerIp( const std::string& ip );
	std::string getPeerIp() const;

	// 服务名称
    void setServiceName( const std::string& serviceName );
    std::string getServiceName() const;

	// 本机历史状态
	void setUsedState( const int8_t state );
	int8_t getUsedState() const;

	// 本机当前状态
	void setCurrentState( const int8_t state );
	int8_t getCurrentState() const;

	// 本机成为master的时间戳
	void setMasterTimeStamp( const int32_t timeStamp );
	int32_t getMasterTimeStamp() const;

    // userDefined 用户自定义, 可以用于区分集群
    void setUserDefined( const std::string& userDefined );
    std::string getUserDefined() const;

	//以下为mysql相关配置
	//master的binlog文件名与pos
	void setMasterBinlogName( const std::string& fileName );
	std::string getMasterBinLogName() const;

	void setMasterBinlogPos( const int32_t pos );
	int32_t getMasterBinlogPos() const;

	//备机的同步状态与sql状态
	//这两个状态现在备机不用传到主机暂时先保留防止后续需要使用
	//void setSlaveSqlState( const int32_t sqlState );
	//int32_t getSlaveSqlState() const;

	//void setSlaveSyncState( const int32_t syncState );
	//int32_t getSlaveSyncState() const;

	//底层mysql的主备状态
	//void setMysqlMode(const int32_t mysqlMode);
	//int32_t getMysqlMode() const;

	//设置主机初始化的binlog文件名与pos
	void setMasterInitBinlogName( const std::string& fileName );
	std::string getMasterInitBinlogName() const;

	void setMasterInitBinlogPos( const int32_t pos );
	int32_t getMasterInitBinlogPos() const;

	//主机在成为主时是合法成为主还是不合法
	void setMasterChangeFlag( const int32_t flag );
	int32_t getMasterChangeFlag() const;

	void setMasterCurrentBinlog(const std::string& fileName, const int64_t pos, const uint64_t currentTime);
	std::string getMasterCurrentBinLogName() const;
	int32_t getMasterCurrentBinlogPos() const;
	uint64_t getMasterCurrentBinlogTime() const;

private:
	class Internel;
	Internel *m_internel;

};

typedef Memory::TSmartObjectPtr<CMulticastHeartBeatMessage> TMulticastHeatBeatMsgPtr;

EFS::TPacketBasePtr newMulticastHeartBeatMessageRequest();
EFS::TPacketBasePtr newMulticastHeartBeatMessageResponse();

} // DBHA
} // VideoCloud
} // Dahua

#endif // __DAHUA_VIDEOCLOUD_DBHA_HEARTBEATMESSAGE_H__

