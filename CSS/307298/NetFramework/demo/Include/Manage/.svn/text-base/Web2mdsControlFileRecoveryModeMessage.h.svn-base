//
//
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		2013年10月28日  23760  Modification
//

#ifndef __INCLUDE_DAHUA_WEB2MDS_CONTROL_FILE_RECOVERY_MODE_MESSAGE_H__
#define __INCLUDE_DAHUA_WEB2MDS_CONTROL_FILE_RECOVERY_MODE_MESSAGE_H__
#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {

//类名 	   CWeb2mdsControlFileRecoveryModeMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   WEB2MDS_CONTROL_FILE_RECOVERY_MODE_MESSAGE
//发起方   运维
//接收方   MDS
//描述	   运维向MDS发送控制文件恢复模式的命令

//类名 	   CResWeb2mdsControlFileRecoveryModeMessage
//协议类型 DS_PROTOCOL_RES
//协议名   WEB2MDS_CONTROL_FILE_RECOVERY_MODE_MESSAGE
//发起方   MDS
//接收方   运维
//描述	   正常,MDS向运维答复当前命令执行情况

enum FileRecoveryMode
{
     manaual = 0, automation = 1, query = 2,nonemode = 3,
};

class CWeb2mdsControlFileRecoveryModeMessage: public CPacketBase
{
public:
    CWeb2mdsControlFileRecoveryModeMessage();
    CWeb2mdsControlFileRecoveryModeMessage( uint32_t seq );
    ~CWeb2mdsControlFileRecoveryModeMessage();

    //尾部插入方式增加原始数据( 序列化后的 )
    bool append( const char* buf, uint32_t len );
    //获取序列化的数据
    const char* getData();
    //获取序列化的数据的长度
    uint32_t getDataLen();
    //打印包用于debug
    void printPacket() const;

    void setModeState( FileRecoveryMode mode );
    FileRecoveryMode getModeState() const;
private:
    class Internel;
    Internel* m_internel;
};

class CResWeb2mdsControlFileRecoveryModeMessage: public CPacketBase
{
public:
    CResWeb2mdsControlFileRecoveryModeMessage();
    CResWeb2mdsControlFileRecoveryModeMessage( uint32_t seq );
    ~CResWeb2mdsControlFileRecoveryModeMessage();

    //尾部插入方式增加原始数据( 序列化后的 )
    bool append( const char* buf, uint32_t len );
    //获取序列化的数据
    const char* getData();
    //获取序列化的数据的长度
    uint32_t getDataLen();
    //打印包用于debug
    void printPacket() const;

    //协议内容
    //包有错误,则设置错误类型进去,否则默认包是正确的
    void setErrorType( int32_t errorType );
    //获取包是否有错,0表示正确,-1标识错误
    int8_t isError() const;
    //如果包出错,则获取到的错误类型标识是什么错误
    int32_t getErrorType() const;

    void setModeState( FileRecoveryMode mode );
    FileRecoveryMode getModeState() const;
private:
    class Internel;
    Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CWeb2mdsControlFileRecoveryModeMessage> TWeb2mdsControlFileRecoveryModeMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2mdsControlFileRecoveryModeMessage> TResWeb2mdsControlFileRecoveryModeMsgPtr;

TPacketBasePtr newCWeb2mdsControlFileRecoveryModeMessage();
TPacketBasePtr newCResWeb2mdsControlFileRecoveryModeMessage();

} /* namespace EFS */
} /* namespace Dahua */
#endif /* __INCLUDE_DAHUA_WEB2MDS_CONTROL_FILE_RECOVERY_MODE_MESSAGE_H__ */
