//
//
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		2013年10月28日  23760  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_WEB2MDS_ADD_DATANODE_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_WEB2MDS_ADD_DATANODE_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {

//类名 	   CMdatanodeAdd
//协议类型    DS_PROTOCOL_REQ
//协议名         ADD_DATANODE_MESSAGE
//发起方        运维
//接收方   MDS
//描述	   运维向MDS发送允许datanode注册的消息

//类名 	   CResMdatanodeAdd
//协议类型    DS_PROTOCOL_RES
//协议名         ADD_DATANODE_MESSAGE
//发起方   MDS
//接收方   运维
//描述	   正常,MDS向运维答复是否允许datanode注册成功的消息

class CWeb2mdsAddDatanodeMessage: public CPacketBase
{
public:
	CWeb2mdsAddDatanodeMessage();
	CWeb2mdsAddDatanodeMessage(uint32_t seq);
	~CWeb2mdsAddDatanodeMessage();

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
private:
	class Internel;
	Internel* m_internel;
};

class CResWeb2mdsAddDatanodeMessage: public CPacketBase
{
public:
	CResWeb2mdsAddDatanodeMessage();
	CResWeb2mdsAddDatanodeMessage(uint32_t seq);
	~CResWeb2mdsAddDatanodeMessage();

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

	//设置和获取datanodeip列表
	void setDataNodeIp(std::vector<std::string>& dnIp);
	void getDataNodeIp(std::vector<std::string>& dnIp) const;

private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CWeb2mdsAddDatanodeMessage> TWeb2mdsAddDatanodeMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2mdsAddDatanodeMessage> TResWeb2mdsAddDatanodeMsgPtr;

TPacketBasePtr newCWeb2mdsAddDatanodeMessage();
TPacketBasePtr newCResWeb2mdsAddDatanodeMessage();

} /* namespace EFS */
} /* namespace Dahua */
#endif /* __INCLUDE_DAHUA_EFS_WEB2MDS_ADD_DATANODE_MESSAGE_H__ */
