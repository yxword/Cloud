//
//
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_WEB2MDS_EFS_FILE_INFO_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_WEB2MDS_EFS_FILE_INFO_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {
//类名 	   CMefsFileInfo
//协议类型    DS_PROTOCOL_REQ
//协议名         EFS_FILE_INFO
//发起方         运维
//接收方         MDS
//描述	        运维向MDS获取EFS文件系统文件统计信息

//类名 	   CResMefsFileInfo
//协议类型    DS_PROTOCOL_RES
//协议名         EFS_FILE_INFO
//发起方         MDS
//接收方         运维
//描述	        正常,MDS向运维反馈MDSEFS文件系统文件统计信息

enum fileRecoverState{
    fileNormal = 0,
    fileReadyForRec = 1,
    fileRecovering =2
};
typedef struct EfsFileInfo
{
    uint32_t cTime;
    uint32_t recoverableFiles;
    uint32_t unRecoverableFiles;
    fileRecoverState state;
    EfsFileInfo();
    int length();
    bool Serialize(Serialization& sn);
    bool Deserialize(Serialization& sn);
} EfsFileInfo;

class CWeb2mdsEfsFileInfoMessage: public CPacketBase
{
public:
    CWeb2mdsEfsFileInfoMessage();
    CWeb2mdsEfsFileInfoMessage(uint32_t seq);
    ~CWeb2mdsEfsFileInfoMessage();

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

class CResWeb2mdsEfsFileInfoMessage: public CPacketBase
{
public:
    CResWeb2mdsEfsFileInfoMessage();
    CResWeb2mdsEfsFileInfoMessage(uint32_t seq);
    ~CResWeb2mdsEfsFileInfoMessage();

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

    void getEfsFileInfo(std::vector<struct EfsFileInfo>& efsFileInfo) const;
    void setEfsFileInfo(std::vector<struct EfsFileInfo>& efsFileInfo);

private:
    class Internel;
    Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CWeb2mdsEfsFileInfoMessage> TWeb2mdsEfsFileInfoMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2mdsEfsFileInfoMessage> TResWeb2mdsEfsFileInfoMsgPtr;

TPacketBasePtr newCWeb2mdsEfsFileInfoMessage();
TPacketBasePtr newCResWeb2mdsEfsFileInfoMessage();

} /* namespace EFS */
} /* namespace Dahua */
#endif /* __INCLUDE_DAHUA_EFS_WEB2MDS_EFS_FILE_INFO_MESSAGE_H__ */
