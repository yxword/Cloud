//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_WEB2CS_SET_CLUSTER_NETWORK_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_WEB2CS_SET_CLUSTER_NETWORK_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include <string>

namespace Dahua {
namespace EFS {

//协议类型 DS_PROTOCOL_REQ
//协议名   WEB2CS_SET_CLUSTER_NETWORK_MESSAGE
//发起方   目录服务器
//接收方   运维
//描述     响应是否设置成功

//协议类型 DS_PROTOCOL_RES
//协议名   WEB2CS_SET_CLUSTER_NETWORK_MESSAGE
//发起方   运维
//接收方   目录服务器
//描述     响应目录服务器的网络配置
                 
class CWeb2csSetClusterNetworkMessage : public CPacketBase
{
public:
    CWeb2csSetClusterNetworkMessage( );
    CWeb2csSetClusterNetworkMessage( uint32_t seq );
    ~CWeb2csSetClusterNetworkMessage( );

    //尾部插入方式增加原始数据( 序列化后的 )
    bool append( const char* buf, uint32_t len );
    //获取序列化的数据
    const char* getData( );
    //获取序列化的数据的长度
    uint32_t getDataLen( );
    //打印包用于debug
    void printPacket( ) const;

    //设置/获取目录服务器虚ip
    void setCatalogVip(const std::string& catalogVip);
    std::string getCatalogVip() const;

    //设置/获取目录服务器虚ip的网关
    void setCatalogVipGateWay(const std::string& catalogVipGateWay);
    std::string getCatalogVipGateWay() const;

    //设置/获取目录服务器虚ip的掩码
    void setCatalogVipMask(const std::string& catalogVipMask);
    std::string getCatalogVipMask() const;

    //配置目录服务需要连接的mds的虚ip地址
    void setMdsVip(const std::string& mdsVip);
    std::string getMdsVip() const;

    //配置目录服务器的本地ip地址
    void setLocalIp(const std::string& localIP);
    std::string getLocalIp() const;

    //配置目录服务器的对端ip地址
    void setPeerIp(const std::string& peerIp);
    std::string getPeerIp() const;

    //配置获取对外的实ip
    void setRealIp(const std::string& realIp);
    std::string getRealIp() const;

    //设置/获取目录服务器本地ip的网关
    void setLocalGateWay(const std::string& localGateWay);
    std::string getLocalGateWay() const;

    //设置/获取目录服务器本地ip的掩码
    void setLocalMask(const std::string& localMask);
    std::string getLocalMask() const;    

private:
    class Internel;
    Internel *m_internel;
};


class CResWeb2csSetClusterNetworkMessage : public CPacketBase
{
public:
    CResWeb2csSetClusterNetworkMessage( );
    CResWeb2csSetClusterNetworkMessage( uint32_t seq );
    ~CResWeb2csSetClusterNetworkMessage( );

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

private:
    class Internel; 
    Internel *m_internel;
};  

typedef Memory::TSmartObjectPtr<CWeb2csSetClusterNetworkMessage> TWeb2csSetClusterNetworkMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2csSetClusterNetworkMessage> TResWeb2csSetClusterNetworkMsgPtr;
TPacketBasePtr newCWeb2csSetClusterNetworkMessage(  );
TPacketBasePtr newCResWeb2csSetClusterNetworkMessage(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_WEB2CS_SET_CLUSTER_NETWORK_MESSAGE_H__
