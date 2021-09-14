//
//  "$Id: DatanodeDetail.h  2013年12月19日  zhang_hailong 23919 $"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	定义运维查询Datanode详细信息的请求和响应报文
//	Revisions:		Year-Month-Day     SVN-Author         Modification
//					2013年12月19日	   zhang_hailong	  Create
//

#ifndef __INCLUDE_DAHUA_EFS_WEB2DN_GET_DATANODE_DETAIL_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_WEB2DN_GET_DATANODE_DETAIL_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {

// 定义运维查询Datanode详细信息的请求报文
class CWeb2dnGetDatanodeDetailMessage : public CPacketBase
{
public:
    CWeb2dnGetDatanodeDetailMessage();
    virtual ~CWeb2dnGetDatanodeDetailMessage();

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

/////////////////////////////////////////////////////////////////////////////
// 定义运维查询Datanode详细信息的响应报文
class CResWeb2dnGetDatanodeDetailMessage : public CPacketBase
{
public:
    CResWeb2dnGetDatanodeDetailMessage();
    virtual ~CResWeb2dnGetDatanodeDetailMessage();

    //尾部插入方式增加原始数据( 序列化后的 )
    bool append(const char* buf, uint32_t len);
    //获取序列化的数据
    const char* getData();
    //获取序列化的数据的长度
    uint32_t getDataLen();
    //打印包用于debug
    void printPacket() const;

    // Datanode 服务器开机运行的时间,单位 s
    void setUptime(uint32_t uptime);
    uint32_t getUptime() const;

    // usage[0] : 表示各个CPU使用率总和;
    // usage[1]~usage[N] : 分别表示CPU 1~N 的使用率
    void  setCpuUsage(const std::vector<double>& usage);
    std::vector<double> getCpuUsage() const;

    void setMemoryTotal(int32_t  total);
    int32_t getMemoryTotal() const;

    void setMemoryFree(int32_t  free);
    int32_t  getMemoryFree() const;

    void setDatanodeIp(const std::string& ip);
    std::string getDatanodeIp() const;

    void setDatanodeID(const std::string& id);
    std::string getDatanodeID() const;

    // 网络接收/发送字节/秒
    // inFlow ： true, 接收字节流;  false, 发送字节流
    void setNetworkFlow(uint32_t bytes, bool inFlow);
    uint32_t getNetworkFlow(bool inFlow) const;

    // 网络发送字节/秒
//    void setNetworkWriteBytes(uint32_t bytes);
//    uint32_t getNetworkWriteBytes() const;

    // IO等待(目前只需写死为0)
    void setIOWait(uint32_t wait);
    uint32_t getIOWait() const;

    // 存储磁盘总空间,单位GB. 精确到小数点后3位
    void setDiskTotal(double total);
    double getDiskTotal() const;
    void setDiskUsed(double used);
    double getDiskUsed() const;

    // 磁盘总数(目前阶段返回槽位个数)
    void setDiskCount(uint32_t num);
    uint32_t getDiskCount() const;

    // 当前坏盘数
    void setBrokenDownDisk(uint32_t num);
    uint32_t getBrokenDownDisk() const;

    // 历史坏盘总数
    void setAllBrokenDownDisk(uint32_t num);
    uint32_t getAllBrokenDownDisk() const;

    // 历史坏盘率
    void setDiskFaultRate(double rate);
    double getDiskFaultRate() const;

    // 节点编号
    void setDataUtilNumber(const std::string& datanodeNum);
    std::string getDataUtilNumber() const;
private:
    class PktInternal;
    PktInternal* m_pkt_internal;
};


typedef Memory::TSmartObjectPtr<CWeb2dnGetDatanodeDetailMessage>   TWeb2dnGetDatanodeDetailMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2dnGetDatanodeDetailMessage> TResWeb2dnGetDatanodeDetailMsgPtr;

TPacketBasePtr newCWeb2dnGetDatanodeDetailMessage();
TPacketBasePtr newCResWeb2dnGetDatanodeDetailMessage();

} /* namespace EFS */
} /* namespace Dahua */
#endif /* __DAHUA_EFS_DATANODEDETAIL_H__ */
