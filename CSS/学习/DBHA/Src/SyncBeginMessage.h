//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:    主备同步开始消息.
//	Revisions:		2017-1-5  mao_jijia
//

#ifndef __DAHUA_VIDEOCLOUD_DBHA_SYNCBEGINMESSAGE_H__
#define __DAHUA_VIDEOCLOUD_DBHA_SYNCBEGINMESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

//同步协议采用二进制协议，因为需要传输文件.
class CSyncBeginMessage : public EFS::CPacketBase
{
public:
	CSyncBeginMessage();
	CSyncBeginMessage( uint32_t seq );
	~CSyncBeginMessage();

	//尾部插入方式增加原始数据( 序列化后的 ).
	bool append( const char* buf, uint32_t len );

	//获取序列化的数据.
	const char* getData();

	//获取序列化的数据的长度.
	uint32_t getDataLen();

	//打印包用于debug.
	void printPacket() const;

	void setFileInfo(std::string fileName, uint64_t pos);

	//获取文件数据.
	void getFileInfo(std::string& fileName, uint64_t& pos);

	//设置文件数据.
	void setMd5sum(std::string &md5sum);

	//获取文件数据.
	void getMd5sum(std::string &md5sum);

private:
	class Internal;
	Internal* m_internal;
};

class CResSyncBeginMessage : public EFS::CPacketBase
{
public:
	CResSyncBeginMessage();
	CResSyncBeginMessage( uint32_t seq );
	~CResSyncBeginMessage();	

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

typedef Memory::TSmartObjectPtr<CSyncBeginMessage> TSyncBeginMsgPtr;
typedef Memory::TSmartObjectPtr<CResSyncBeginMessage> TResSyncBeginMsgPtr;
EFS::TPacketBasePtr newCSyncBeginMessage();
EFS::TPacketBasePtr newCResSyncBeginMessage();


} // DBHA
} // VideoCloud
} // Dahua

#endif // __DAHUA_VIDEOCLOUD_DBHA_SYNCBEGINMESSAGE_H__
