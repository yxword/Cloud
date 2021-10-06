//
//
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_WEB2MDS_GET_MDSIP_INFO_MESSAGE_H__
#define __INCLUDE_DAHUA_WEB2MDS_GET_MDSIP_INFO_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {
//类名 	   CWeb2mdsGetMdsIpInfoMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   WEB2MDS_GET_MDS_IP_INFO_MESSAGE
//发起方   客户端
//接收方   MDS
//描述	   客户端向MDS获取MDS信息

//类名 	   CResWeb2mdsGetMdsIpInfoMessage
//协议类型 DS_PROTOCOL_RES
//协议名   WEB2MDS_GET_MDS_IP_INFO_MESSAGE
//发起方   MDS
//接收方   客户端
//描述	   正常,MDS向客户端MDS信息

class CWeb2mdsGetMdsIpInfoMessage: public CPacketBase
{
public:
	CWeb2mdsGetMdsIpInfoMessage();
	CWeb2mdsGetMdsIpInfoMessage(uint32_t seq);
	~CWeb2mdsGetMdsIpInfoMessage();

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append(const char* buf, uint32_t len);
	//获取序列化的数据
	const char* getData();
	//获取序列化的数据的长度
	uint32_t getDataLen();
	//打印包用于debug
	void printPacket() const;

private:
	class Internel;
	Internel* m_internel;
};

class CResWeb2mdsGetMdsIpInfoMessage: public CPacketBase
{
public:
	CResWeb2mdsGetMdsIpInfoMessage();
	CResWeb2mdsGetMdsIpInfoMessage(uint32_t seq);
	~CResWeb2mdsGetMdsIpInfoMessage();

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

	const std::string& getMdsLocalIp() const;
	void setMdsLocalIp(const std::string& mdsLocalIp);
		
	const std::string& getMdsRemoteIp() const;
	void setMdsRemoteIp(const std::string& mdsRemoteIp);	
		
	const std::string& getMdsLocalGatewayIp() const;
	void setMdsLocalGatewayIp(const std::string& mdsLocalGatewayIp);
		
	const std::string& getMdsLocalMaskIp() const;
	void setMdsLocalMaskIp(const std::string& mdsLocalMaskIp);

	const std::string& getMdsRealIp() const;
	void setMdsRealIp(const std::string& mdsRealIp);

	const std::string& getMdsVirtualIp() const;
	void setMdsVirtualIp(const std::string& mdsVirtualIp);

	const std::string& getMdsRealGatewayIp() const;
	void setMdsRealGatewayIp(const std::string& mdsRealGatewayIp);

	const std::string& getMdsRealMaskIp() const;
	void setMdsRealMaskIp(const std::string& mdsRealMaskIp);


private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CWeb2mdsGetMdsIpInfoMessage> TWeb2mdsGetMdsIpInfoMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2mdsGetMdsIpInfoMessage> TResWeb2mdsGetMdsIpInfoMsgPtr;

TPacketBasePtr newCWeb2mdsGetMdsIpInfoMessage();
TPacketBasePtr newCResWeb2mdsGetMdsIpInfoMessage();

} /* namespace EFS */
} /* namespace Dahua */
#endif /* __INCLUDE_DAHUA_WEB2MDS_GET_MDSIP_INFO_MESSAGE_H__ */
