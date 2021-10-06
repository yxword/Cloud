//
//
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		2013年10月28日  23760  Modification
//

#ifndef __INCLUDE_DAHUA_WEB2MDS_SYSTEM_CONTROL_INFO_MESSAGE_H__
#define __INCLUDE_DAHUA_WEB2MDS_SYSTEM_CONTROL_INFO_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {

//类名 	   CMsystemControl
//协议类型 DS_PROTOCOL_REQ
//协议名   SYSTEM_CONTROL_INFO
//发起方   客户端
//接收方   MDS
//描述	   客户端向MDS发送系统启停命令

//类名 	   CResMsystemControl
//协议类型 DS_PROTOCOL_RES
//协议名   SYSTEM_CONTROL_INFO
//发起方   MDS
//接收方   客户端
//描述	   正常,MDS向客户端答复当前服务状态

enum SystemService{
	off =0,
	on= 1,
	none =2
};

class CWeb2mdsSystemControlInfoMessage : public CPacketBase{
public:
	CWeb2mdsSystemControlInfoMessage();
	CWeb2mdsSystemControlInfoMessage( uint32_t seq );
	~CWeb2mdsSystemControlInfoMessage();

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	void setServiceState(SystemService state);
	SystemService getServiceState() const;
private:
	class Internel;
	Internel* m_internel;
};

class CResWeb2mdsSystemControlInfoMessage: public CPacketBase{
public:
	CResWeb2mdsSystemControlInfoMessage();
	CResWeb2mdsSystemControlInfoMessage(uint32_t seq);
	~CResWeb2mdsSystemControlInfoMessage();

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

	void setServiceState(SystemService state);
	SystemService getServiceState() const;
private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CWeb2mdsSystemControlInfoMessage> TWeb2mdsSystemControlInfoMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2mdsSystemControlInfoMessage> TResWeb2mdsSystemControlInfoMsgPtr;

TPacketBasePtr newCWeb2mdsSystemControlInfoMessage( );
TPacketBasePtr newCResWeb2mdsSystemControlInfoMessage( );

} /* namespace EFS */
} /* namespace Dahua */
#endif /* __INCLUDE_DAHUA_WEB2MDS_SYSTEM_CONTROL_INFO_MESSAGE_H__ */
