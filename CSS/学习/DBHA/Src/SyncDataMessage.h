//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:    主备同步发送数据库文件的消息.
//	Revisions:		2017-1-5  mao_jijia
//

#ifndef __DAHUA_VIDEOCLOUD_DBHA_SYNCDATAMESSAGE_H__
#define __DAHUA_VIDEOCLOUD_DBHA_SYNCDATAMESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

//同步协议采用二进制协议，因为需要传输文件.
class CSyncDataMessage : public EFS::CPacketBase
{
public:
	CSyncDataMessage();
	CSyncDataMessage( uint32_t seq );
	~CSyncDataMessage();

	//尾部插入方式增加原始数据( 序列化后的 ).
	bool append( const char* buf, uint32_t len );

	//获取序列化的数据.
	const char* getData();

	//获取序列化的数据的长度.
	uint32_t getDataLen();

	//打印包用于debug.
	void printPacket() const;

	//设置文件数据.
	bool setSyncMessage( char* msg, int32_t len );

	//获取文件数据.
	bool getSyncMessage( char** msg, int32_t& len );

private:
	class Internal;
	Internal* m_internal;
};

class CResSyncDataMessage : public EFS::CPacketBase
{
public:
	CResSyncDataMessage();
	CResSyncDataMessage( uint32_t seq );
	~CResSyncDataMessage();	

	//尾部插入方式增加原始数据( 序列化后的 ).
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据.
	const char* getData();
	//获取序列化的数据的长度.
	uint32_t getDataLen();
	//打印包用于debug.
	void printPacket() const;

	//协议内容
	//包有错误,则设置错误类型进去,否则默认包是正确的.
	void setErrorType( int32_t errorType );
	//获取包是否有错,0表示正确,-1标识错误.
	int8_t isError( ) const;
	//如果包出错,则获取到的错误类型标识是什么错误.
	int32_t getErrorType( ) const;

private:
	class Internal;
	Internal* m_internal;	
};

typedef Memory::TSmartObjectPtr<CSyncDataMessage> TSyncDataMsgPtr;
typedef Memory::TSmartObjectPtr<CResSyncDataMessage> TResSyncDataMsgPtr;
EFS::TPacketBasePtr newCSyncDataMessage();
EFS::TPacketBasePtr newCResSyncDataMessage();


} // DBHA
} // VideoCloud
} // Dahua

#endif // __DAHUA_VIDEOCLOUD_DBHA_SYNCDATAMESSAGE_H__
