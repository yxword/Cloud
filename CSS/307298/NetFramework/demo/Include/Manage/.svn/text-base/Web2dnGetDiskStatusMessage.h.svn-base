//
//  "$Id: DnGetDiskStatus.h  2013年12月19日  zhang_hailong 23919 $"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	定义运维向Datanode查询所有磁盘状态的请求和响应报文
//	Revisions:		Year-Month-Day     SVN-Author         Modification
//					2013年12月19日	   zhang_hailong	  Create
//

#ifndef __INCLUDE_DAHUA_EFS_WEB2DN_GET_DISK_STATUS_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_WEB2DN_GET_DISK_STATUS_MESSAGE_H__

#include <vector>
#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {

// 运维向Datanode查询磁盘状态的请求报文
class CWeb2dnGetDiskStatusMessage: public CPacketBase {
public:
    CWeb2dnGetDiskStatusMessage();
    virtual ~CWeb2dnGetDiskStatusMessage();

    //尾部插入方式增加原始数据(序列化后的 )
    bool append(const char* buf, uint32_t len);
    //获取序列化的数据
    const char* getData();
    //获取序列化的数据的长度
    uint32_t getDataLen();
    //打印包用于debug
    void printPacket() const;

private:
    class PktInternal;
    PktInternal* m_pkt_internal;
};


///////////////////////////////////////////////////////////////////////////////
// 运维向Datanode查询磁盘状态的响应报文
class CResWeb2dnGetDiskStatusMessage: public CPacketBase {
public:
    CResWeb2dnGetDiskStatusMessage();
    virtual ~CResWeb2dnGetDiskStatusMessage();

    //尾部插入方式增加原始数据(序列化后的 )
    bool append(const char* buf, uint32_t len);
    //获取序列化的数据
    const char* getData();
    //获取序列化的数据的长度
    uint32_t getDataLen();
    //打印包用于debug
    void printPacket() const;

    void setDiskStatus(const std::vector<int>& diskStatus);
    std::vector<int> getDiskStatus() const;

private:
    class PktInternal;
    PktInternal* m_pkt_internal;
};


typedef Memory::TSmartObjectPtr<CWeb2dnGetDiskStatusMessage>   TWeb2dnGetDiskStatusMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2dnGetDiskStatusMessage> TResWeb2dnGetDiskStatusMsgPtr;

TPacketBasePtr newCWeb2dnGetDiskStatusMessage();
TPacketBasePtr newCResWeb2dnGetDiskStatusMessage();

} /* namespace EFS */
} /* namespace Dahua */
#endif /* __DAHUA_EFS_WEB2DN_GET_DISK_STATUS_MESSAGE_H__ */
