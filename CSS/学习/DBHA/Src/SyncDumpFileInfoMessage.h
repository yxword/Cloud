//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:    主备同步获取版本号的消息.
//	Revisions:		2017-1-5  mao_jijia
//

#ifndef __DAHUA_VIDEOCLOUD_DBHA_SYNCDUMPFILEINFOMESSAGE_H__
#define __DAHUA_VIDEOCLOUD_DBHA_SYNCDUMPFILEINFOMESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace VideoCloud {
namespace DBHA {


//同步协议采用二进制协议，因为需要传输文件.
class CSyncDumpFileInfoMessage : public EFS::CPacketBase
{
public:
	CSyncDumpFileInfoMessage();
	CSyncDumpFileInfoMessage( uint32_t seq );
	~CSyncDumpFileInfoMessage();

	//尾部插入方式增加原始数据( 序列化后的 ).
	bool append( const char* buf, uint32_t len );

	//获取序列化的数据.
	const char* getData();

	//获取序列化的数据的长度.
	uint32_t getDataLen();

	//打印包用于debug.
	void printPacket() const;

	//设置版本号.
	//void setDumFileInfo( uint64_t version );

	//获取Json协议.
	//void getDumpFileInfo( uint64_t& version );

private:
	class Internal;
	Internal* m_internal;
};

class CResSyncDumpFileInfoMessage : public EFS::CPacketBase
{
public:
	CResSyncDumpFileInfoMessage();
	CResSyncDumpFileInfoMessage( uint32_t seq );
	~CResSyncDumpFileInfoMessage();	

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

	//设置版本号.
	void setDumpFileInfo(std::string fileName, uint64_t len);

	//获取Json协议.
	void getDumpFileInfo(std::string& fileName, uint64_t& len);

private:
	class Internal;
	Internal* m_internal;	
};

typedef Memory::TSmartObjectPtr<CSyncDumpFileInfoMessage> TSyncDumpFileInfoMsgPtr;
typedef Memory::TSmartObjectPtr<CResSyncDumpFileInfoMessage> TResSyncDumpFileInfoMsgPtr;
EFS::TPacketBasePtr newCSyncDumpFileInfoMessage();
EFS::TPacketBasePtr newCResSyncDumpFileInfoMessage();

} // DBHA
} // VideoCloud
} // Dahua

#endif // __DAHUA_VIDEOCLOUD_DBHA_SYNCDUMPFILEINFOMESSAGE_H__
