//
//
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_WEB2MDS_GET_MDS_INFO_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_WEB2MDS_GET_MDS_INFO_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {
//类名 	   CMmdsInfo
//协议类型 DS_PROTOCOL_REQ
//协议名   GET_MDS_INFO
//发起方   客户端
//接收方   MDS
//描述	   客户端向MDS获取MDS信息

//类名 	   CResMmdsInfo
//协议类型 DS_PROTOCOL_RES
//协议名   GET_MDS_INFO
//发起方   MDS
//接收方   客户端
//描述	   正常,MDS向客户端MDS信息

class CWeb2mdsGetMdsInfoMessage: public CPacketBase
{
public:
	CWeb2mdsGetMdsInfoMessage();
	CWeb2mdsGetMdsInfoMessage(uint32_t seq);
	~CWeb2mdsGetMdsInfoMessage();

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

class CResWeb2mdsGetMdsInfoMessage: public CPacketBase
{
public:
    CResWeb2mdsGetMdsInfoMessage();
    CResWeb2mdsGetMdsInfoMessage(uint32_t seq);
	~CResWeb2mdsGetMdsInfoMessage();

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

	uint8_t getClusterId() const;
	void setClusterId(uint8_t clusterId);
	const std::vector<int16_t>& getCpuLoad();
	void setCpuLoad(const std::vector<int16_t>& cpuLoad);
	int16_t getLiveSdks() const;
	void setLiveSdks(int16_t liveSdks);

	const std::string& getMdsIp() const;
	void setMdsIp(const std::string& mdsIp);

	const std::string& getStandbyMdsIp() const;
	void setStandbyMdsIp(const std::string& standbyMdsIp);

	const std::string& getMdsName() const;
	void setMdsName(const std::string& mdsName);

	const std::string& getStandbyMdsName() const;
	void setStandbyMdsName(const std::string& standbyMdsName);

	uint8_t getMdsState() const;
	void setMdsState(uint8_t mdsState);

	uint8_t getStandbyMdsState() const;
	void setStandbyMdsState(uint8_t standbyMdsState);

	uint8_t getMdsType() const;
	void setMdsType(uint8_t mdsType);

	int16_t getMemoryLoad() const;
	void setMemoryLoad(int16_t memoryLoad);

	int64_t getClusterInThroughput() const;
	void setClusterInThroughput(int64_t inThroughput);

	int64_t getClusterOutThroughput() const;
	void setClusterOutThroughput(int64_t outThroughput);

private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CWeb2mdsGetMdsInfoMessage> TWeb2mdsGetMdsInfoMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2mdsGetMdsInfoMessage> TResWeb2mdsGetMdsInfoMsgPtr;

TPacketBasePtr newCWeb2mdsGetMdsInfoMessage();
TPacketBasePtr newCResWeb2mdsGetMdsInfoMessage();

} /* namespace EFS */
} /* namespace Dahua */
#endif /* __INCLUDE_DAHUA_EFS_WEB2MDS_GET_MDS_INFO_MESSAGE_H__ */
