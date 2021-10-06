//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_WEB2CS_GET_CATALOG_INFO_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_WEB2CS_GET_CATALOG_INFO_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include <string>
#include <vector>

namespace Dahua {
namespace EFS {
//协议类型 DS_PROTOCOL_REQ
//协议名   WEB2CS_GET_CATALOG_INFO_MESSAGE
//发起方   目录服务器
//接收方   运维
//描述     查询目录服务器的状态与统计信息

//协议类型 DS_PROTOCOL_REQ
//协议名   WEB2CS_GET_CATALOG_INFO_MESSAGE
//发起方   运维
//接收方   目录服务器
//描述     返回目录服务器的状态与统计信息

class CWeb2csGetCatalogInfoMessage : public CPacketBase
{
public:
    CWeb2csGetCatalogInfoMessage( );
    CWeb2csGetCatalogInfoMessage( uint32_t seq );
    ~CWeb2csGetCatalogInfoMessage( );

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
    Internel *m_internel;
};


class CResWeb2csGetCatalogInfoMessage : public CPacketBase
{
public:
    CResWeb2csGetCatalogInfoMessage( );
    CResWeb2csGetCatalogInfoMessage( uint32_t seq );
    ~CResWeb2csGetCatalogInfoMessage( );

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

    //设置获取cpu负载，第一个是cpu总的负载信息
    void setCpuLoad(const std::vector<int16_t>& cpuLoads);
    std::vector<int16_t> getCpuLoad() const;

    //设置获取内存负载
    void setMemoryLoad(const int16_t memoryLoad);
    int16_t getMemoryLoad() const;

    //设置获取主目录服务器的ip
    void setMasterIp(const std::string& ip);
    std::string getMasterIp() const;
    
    //设置获取主目录服务器的状态
    //0正常，1只读，2恢复，3离线。主有正常，只读，离线三个状态
    void setMasterStatus(const int16_t status);
    int16_t getMasterStatus() const;

    //设置获取备目录服务器的ip
    void setBackupIp(const std::string& ip);
    std::string getBackupIp() const;

    //设置获取备目录服务器的状态
    //0正常，1只读，2恢复，3离线。备有正常，恢复，离线三个状态
    void setBackupStatus(const int16_t status);
    int16_t getBackupStatus() const;

    //设置获取bucket总数
    void setBucketNum(const uint64_t bucketNum);
    uint64_t getBucketNum()const ;

    //设置获取文件总数
    void setTotalFiles(const uint64_t files);
    uint64_t getTotalFiles()const;
private:
    class Internel; 
    Internel *m_internel;
};  

typedef Memory::TSmartObjectPtr<CWeb2csGetCatalogInfoMessage> TWeb2csGetCatalogInfoMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2csGetCatalogInfoMessage> TResWeb2csGetCatalogInfoMsgPtr;
TPacketBasePtr newCWeb2csGetCatalogInfoMessage(  );
TPacketBasePtr newCResWeb2csGetCatalogInfoMessage(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_WEB2CS_GET_CATALOG_INFO_MESSAGE_H__
