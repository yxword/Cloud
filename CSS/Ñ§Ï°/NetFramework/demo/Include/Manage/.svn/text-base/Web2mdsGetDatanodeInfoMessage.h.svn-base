//
//
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		2013年10月28日  23760  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_WEB2MDS_GET_DATANODE_INFO_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_WEB2MDS_GET_DATANODE_INFO_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {

//类名 	   CMdatanodeInfo
//协议类型    DS_PROTOCOL_REQ
//协议名         GET_DATANODE_INFO
//发起方   运维
//接收方   MDS
//描述	   运维向MDS发送查询集群datanode的命令

//类名 	   CResMdatanodeInfo
//协议类型    DS_PROTOCOL_RES
//协议名         GET_DATANODE_INFO
//发起方   MDS
//接收方   运维
//描述	   正常,MDS向运维答复当前集群datanode的状态信息

enum DataNodeState
{
	offline = 0, online = 1, total = 2
};

typedef struct DataNodeStatus{
    std::string dnName;
    std::string dnIp;
    //uint16_t dnId
    uint8_t dnState;
    int64_t dnCapacity;
    int64_t dnRemainCapacity;
    int64_t dnRIo;
    int64_t dnWIo;
    int16_t dnConnect;
    int16_t dnHeartBeat;
  //  std::vector<int16_t> cpuLoad;
   // int16_t memoryLoad;
    int8_t dnBadDisk;
    DataNodeStatus();
    int length();
    bool Serialize(Serialization& sn);
    bool Deserialize(Serialization& sn);
}DataNodeStatus;

class CWeb2mdsGetDatanodeInfoMessage: public CPacketBase
{
public:
	CWeb2mdsGetDatanodeInfoMessage();
	CWeb2mdsGetDatanodeInfoMessage(uint32_t seq);
	~CWeb2mdsGetDatanodeInfoMessage();

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append(const char* buf, uint32_t len);
	//获取序列化的数据
	const char* getData();
	//获取序列化的数据的长度
	uint32_t getDataLen();
	//打印包用于debug
	void printPacket() const;

	void setDataNodeState(DataNodeState state);
	DataNodeState getDataNodeState() const;
private:
	class Internel;
	Internel* m_internel;
};

class CResWeb2mdsGetDatanodeInfoMessage: public CPacketBase
{
public:
	CResWeb2mdsGetDatanodeInfoMessage();
	CResWeb2mdsGetDatanodeInfoMessage(uint32_t seq);
	~CResWeb2mdsGetDatanodeInfoMessage();

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

	//设置和获取datanodestatus列表
	void setDataNodeStatus( std::vector<struct DataNodeStatus>& dnStatus );
	void getDataNodeStatus( std::vector<struct DataNodeStatus>& dnStatus ) const;

private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CWeb2mdsGetDatanodeInfoMessage> TWeb2mdsGetDatanodeInfoMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2mdsGetDatanodeInfoMessage> TResWeb2mdsGetDatanodeInfoMsgPtr;

TPacketBasePtr newCWeb2mdsGetDatanodeInfoMessage();
TPacketBasePtr newCResWeb2mdsGetDatanodeInfoMessage();

} /* namespace EFS */
} /* namespace Dahua */
#endif /* __INCLUDE_DAHUA_EFS_WEB2MDS_GET_DATANODE_INFO_MESSAGE_H__ */
