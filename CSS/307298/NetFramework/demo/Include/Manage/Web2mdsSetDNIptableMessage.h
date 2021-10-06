//
//
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		2013年11月14日  23899  Modification
//

#ifndef __INCLUDE_DAHUA_WEB2MDS_SET_DN_IPTABLE_MESSAGE_H__
#define __INCLUDE_DAHUA_WEB2MDS_SET_DN_IPTABLE_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {

//类名 	   CWeb2mdsSetWebIptableMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   WEB2MDS_SET_DNIPTABLE_MESSAGE
//发起方   客户端
//接收方   MDS
//描述	   客户端向MDS发送配置集群网络的命令

//类名 	   CResWeb2mdsSetWebIptableMessage
//协议类型 DS_PROTOCOL_RES
//协议名   WEB2MDS_SET_DNIPTABLE_MESSAGE
//发起方   MDS
//接收方   客户端
//描述	   正常,MDS向客户端答复当前集群网络状态

class CWeb2mdsSetDNIptableMessage: public CPacketBase
{
public:
	CWeb2mdsSetDNIptableMessage();
	CWeb2mdsSetDNIptableMessage(uint32_t seq);
	~CWeb2mdsSetDNIptableMessage();

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append(const char* buf, uint32_t len);
	//获取序列化的数据
	const char* getData();
	//获取序列化的数据的长度
	uint32_t getDataLen();
	//打印包用于debug
	void printPacket() const;

	// iptable为一行行字符串，格式为
	// ip1:port1-ip2:port2
	// ip1:port1为源地址端口，ip2:port2为映射后地址端口
	void setIptable(const std::string iptable);
	std::string getIptable() const;

private:
	class Internel;
	Internel* m_internel;
};

class CResWeb2mdsSetDNIptableMessage: public CPacketBase
{
public:
	CResWeb2mdsSetDNIptableMessage();
	CResWeb2mdsSetDNIptableMessage(uint32_t seq);
	~CResWeb2mdsSetDNIptableMessage();

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

private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CWeb2mdsSetDNIptableMessage> TWeb2mdsSetDNIptableMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2mdsSetDNIptableMessage> TResWeb2mdsSetDNIptableMsgPtr;

TPacketBasePtr newCWeb2mdsSetDNIptableMessage();
TPacketBasePtr newCResWeb2mdsSetDNIptableMessage();

} /* namespace EFS */
} /* namespace Dahua */
#endif /* CMSYSTEMCONTROL_H_ */
