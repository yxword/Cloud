//
//
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		2013年11月14日  23899  Modification
//

#ifndef __INCLUDE_DAHUA_WEB2MDS_SET_CLUSTER_NETWORK_MESSAGE_H__
#define __INCLUDE_DAHUA_WEB2MDS_SET_CLUSTER_NETWORK_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {

//类名 	   CMsystemControl
//协议类型 DS_PROTOCOL_REQ
//协议名   SET_CLUSTER_NETWORK
//发起方   客户端
//接收方   MDS
//描述	   客户端向MDS发送配置集群网络的命令

//类名 	   CResMsystemControl
//协议类型 DS_PROTOCOL_RES
//协议名   SET_CLUSTER_NETWORK
//发起方   MDS
//接收方   客户端
//描述	   正常,MDS向客户端答复当前集群网络状态

enum ClusterNetworkStatus
{
	abnormal = 0, normal = 1
};

class CWeb2mdsSetClusterNetworkMessage: public CPacketBase
{
public:
	CWeb2mdsSetClusterNetworkMessage();
	CWeb2mdsSetClusterNetworkMessage(uint32_t seq);
	~CWeb2mdsSetClusterNetworkMessage();

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append(const char* buf, uint32_t len);
	//获取序列化的数据
	const char* getData();
	//获取序列化的数据的长度
	uint32_t getDataLen();
	//打印包用于debug
	void printPacket() const;

	void setLocalIp(const std::string ip);
	std::string getLocalIp() const;

	void setLocalIpGateway(const std::string ipGateway);
	std::string getLocalIpGateway() const;

	void setLocalIpMask(const std::string ipMask);
	std::string getLocalIpMask() const;

	void setRealIp(const std::string realIp);
	std::string getRealIp() const;

	void setPeerIp(std::string peerIp);
	std::string getPeerIp() const;

	void setVIp(std::string vIp);
	std::string getVIp() const;

	void setVIpGateway(const std::string vIpGateway);
	std::string getVIpGateway() const;

	void setVIpMask(const std::string vIpMask);
	std::string getVIpMask() const;

private:
	class Internel;
	Internel* m_internel;
};

class CResWeb2mdsSetClusterNetworkMessage: public CPacketBase
{
public:
	CResWeb2mdsSetClusterNetworkMessage();
	CResWeb2mdsSetClusterNetworkMessage(uint32_t seq);
	~CResWeb2mdsSetClusterNetworkMessage();

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append(const char* buf, uint32_t len);
	//获取序列化的数据
	const char* getData();
	//获取序列化的数据的长度
	uint32_t getDataLen();
	//打印包用于debug
	void printPacket() const;

	//协议内容
	//包有错误,则设置错误类型进去,否则默认包是正确的
	void setErrorType(int32_t errorType);
	//获取包是否有错,0表示正确,-1标识错误
	int8_t isError() const;
	//如果包出错,则获取到的错误类型标识是什么错误
	int32_t getErrorType() const;

	void setClusterNetworkStatus(ClusterNetworkStatus state);
	ClusterNetworkStatus getClusterNetworkStatus() const;
private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CWeb2mdsSetClusterNetworkMessage> TWeb2mdsSetClusterNetworkMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2mdsSetClusterNetworkMessage> TResWeb2mdsSetClusterNetworkMsgPtr;

TPacketBasePtr newCWeb2mdsSetClusterNetworkMessage();
TPacketBasePtr newCResWeb2mdsSetClusterNetworkMessage();

} /* namespace EFS */
} /* namespace Dahua */
#endif /* CMSYSTEMCONTROL_H_ */
