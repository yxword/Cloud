//
//
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_WEB2MDS_BLOCK_RESTORE_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_WEB2MDS_BLOCK_RESTORE_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {
//类名 	   CMblockRestore
//协议类型    DS_PROTOCOL_REQ
//协议名         BLOCK_RESTORE_MESSAGE
//发起方         运维
//接收方         MDS
//描述	        运维向MDS下发恢复文件的指令

//类名 	   CResMblockRestore
//协议类型    DS_PROTOCOL_RES
//协议名         BLOCK_RESTORE_MESSAGE
//发起方         MDS
//接收方         运维
//描述	        正常,MDS向运维反馈恢复指令下发情况

class CWeb2mdsBlockRestoreMessage: public CPacketBase
{
public:
	CWeb2mdsBlockRestoreMessage();
	CWeb2mdsBlockRestoreMessage(uint32_t seq);
	~CWeb2mdsBlockRestoreMessage();

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append(const char* buf, uint32_t len);
	//获取序列化的数据
	const char* getData();
	//获取序列化的数据的长度
	uint32_t getDataLen();
	//打印包用于debug
	void printPacket() const;

	void getBlockRestoreMessage(std::vector<uint32_t>& day) const;
	void setBlockRestoreMessage(std::vector<uint32_t>& day);

private:
	class Internel;
	Internel* m_internel;
};

class CResWeb2mdsBlockRestoreMessage: public CPacketBase
{
public:
	CResWeb2mdsBlockRestoreMessage();
	CResWeb2mdsBlockRestoreMessage(uint32_t seq);
	~CResWeb2mdsBlockRestoreMessage();

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

typedef Memory::TSmartObjectPtr<CWeb2mdsBlockRestoreMessage>   TWeb2mdsBlockRestoreMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2mdsBlockRestoreMessage> TResWeb2mdsBlockRestoreMsgPtr;

TPacketBasePtr newCWeb2mdsBlockRestoreMessage();
TPacketBasePtr newCResWeb2mdsBlockRestoreMessage();

} /* namespace EFS */
} /* namespace Dahua */
#endif /* __INCLUDE_DAHUA_EFS_WEB2MDS_BLOCK_RESTORE_MESSAGE_H__ */
