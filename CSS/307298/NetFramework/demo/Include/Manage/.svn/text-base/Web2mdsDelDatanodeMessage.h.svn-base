//
//
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		2013年10月28日  23760  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_WEB2MDS_DEL_DATANODE_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_WEB2MDS_DEL_DATANODE_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {

//类名 	   CMdatanodeDel
//协议类型    DS_PROTOCOL_REQ
//协议名         DEL_DATANODE_MESSAGE
//发起方        运维
//接收方   MDS
//描述	   运维向MDS发送注销datanode的消息

//类名 	   CResMdatanodeDel
//协议类型    DS_PROTOCOL_RES
//协议名         DEL_DATANODE_MESSAGE
//发起方   MDS
//接收方   运维
//描述	   正常,MDS向运维答复是否注销datanode成功的消息

class CWeb2mdsDelDatanodeMessage: public CPacketBase
{
public:
	CWeb2mdsDelDatanodeMessage();
	CWeb2mdsDelDatanodeMessage(uint32_t seq);
	~CWeb2mdsDelDatanodeMessage();

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append(const char* buf, uint32_t len);
	//获取序列化的数据
	const char* getData();
	//获取序列化的数据的长度
	uint32_t getDataLen();
	//打印包用于debug
	void printPacket() const;

	void setDataNodeIp(std::vector<std::string>& dnIp);
	void getDataNodeIp(std::vector<std::string>& dnIp) const;

	void setDataNodeId(std::vector<std::string>& dnId);
	void getDataNodeId(std::vector<std::string>& dnId) const;

private:
	class Internel;
	Internel* m_internel;
};

class CResWeb2mdsDelDatanodeMessage: public CPacketBase
{
public:
	CResWeb2mdsDelDatanodeMessage();
	CResWeb2mdsDelDatanodeMessage(uint32_t seq);
	~CResWeb2mdsDelDatanodeMessage();

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
	void setDataNodeIp(std::vector<std::string>& dnIp);
	void getDataNodeIp(std::vector<std::string>& dnIp) const;

	void setDataNodeId(std::vector<std::string>& dnId);
	void getDataNodeId(std::vector<std::string>& dnId) const;

private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CWeb2mdsDelDatanodeMessage> TWeb2mdsDelDatanodeMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2mdsDelDatanodeMessage> TResWeb2mdsDelDatanodeMsgPtr;

TPacketBasePtr newCWeb2mdsDelDatanodeMessage();
TPacketBasePtr newCResWeb2mdsDelDatanodeMessage();

} /* namespace EFS */
} /* namespace Dahua */
#endif /* __INCLUDE_DAHUA_EFS_WEB2MDS_DEL_DATANODE_MESSAGE_H__ */
