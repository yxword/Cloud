//
//
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_WEB2MDS_GET_SYSTEM_INFO_MESSAGE_H__
#define __INCLUDE_DAHUA_WEB2MDS_GET_SYSTEM_INFO_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {
//类名 	   CMsystemInfo
//协议类型 DS_PROTOCOL_REQ
//协议名   GET_SYSTEM_INFO
//发起方   客户端
//接收方   MDS
//描述	   客户端向MDS获取EFS系统信息

//类名 	   CResMsystemInfo
//协议类型 DS_PROTOCOL_RES
//协议名   GET_SYSTEM_INFO
//发起方   MDS
//接收方   客户端
//描述	   正常,MDS向客户端EFS系统信息

class CWeb2mdsGetSystemInfoMessage: public CPacketBase {
public:
	CWeb2mdsGetSystemInfoMessage();
	CWeb2mdsGetSystemInfoMessage( uint32_t seq );
	~CWeb2mdsGetSystemInfoMessage();

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

private:
	class Internel;
	Internel* m_internel;
};

class CResWeb2mdsGetSystemInfoMessage: public CPacketBase{
public:
	CResWeb2mdsGetSystemInfoMessage();
	CResWeb2mdsGetSystemInfoMessage( uint32_t seq );
	~CResWeb2mdsGetSystemInfoMessage();

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//协议内容
	//包有错误,则设置错误类型进去,否则默认包是正确的
	void setErrorType( int32_t errorType );
	//获取包是否有错,0表示正确,-1标识错误
	int8_t isError( ) const;
	//如果包出错,则获取到的错误类型标识是什么错误
	int32_t getErrorType( ) const;

	int64_t getDAbnormalFiles() const;

	void setDAbnormalFiles(int64_t dAbnormalFiles);

	int64_t getEfsCapacity() const;

	void setEfsCapacity(int64_t efsCapacity);

	int64_t getEfsRemainCapacity() const;

	void setEfsRemainCapacity(int64_t efsRemainCapacity);

	int32_t getLiveNodes() const;

	void setLiveNodes(int32_t liveNodes);

	int64_t getRAbnormalFiles() const ;

	void setRAbnormalFiles(int64_t rAbnormalFiles);

	int32_t getRunTime() const;

	void setRunTime(int32_t runTime);

	std::string getSystemVersion() const;

	void setSystemVersion(std::string systemVersion);

	int64_t getTotalFiles() const;

	void setTotalFiles(int64_t totalFiles);

	int32_t getTotalNodes() const;

	void setTotalNodes(int32_t totalNodes);
private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CWeb2mdsGetSystemInfoMessage> TWeb2mdsGetSystemInfoMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2mdsGetSystemInfoMessage> TResWeb2mdsGetSystemInfoMsgPtr;

TPacketBasePtr newCWeb2mdsGetSystemInfoMessage( );
TPacketBasePtr newCResWeb2mdsGetSystemInfoMessage( );


} /* namespace EFS */
} /* namespace Dahua */
#endif /* __INCLUDE_DAHUA_WEB2MDS_GET_SYSTEM_INFO_MESSAGE_H__ */
