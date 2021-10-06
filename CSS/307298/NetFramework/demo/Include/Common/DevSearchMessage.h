//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_DEV_SEARCH_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_DEV_SEARCH_MESSAGE_H__

#include "Infra/IntTypes.h"
#include <string>

namespace Dahua {
namespace EFS {     

class CDevSearchMessage
{
public:
    CDevSearchMessage( );
    ~CDevSearchMessage( );
    bool getMethod( std::string& method );
    bool getMac( std::string& mac );
private:
    class Internel;
    Internel *m_internel;    
};


class CResDevSearchMessage
{
public:
    CResDevSearchMessage( );
    ~CResDevSearchMessage( );

    //服务端
    void setMethod( const std::string& method );
    void setMac( const std::string& mac );
    void setMachineName( const std::string& machineName );
    void setDeviceClass( const std::string& deviceClass );
    void setDeviceType( const std::string& deviceType );
    void setSerialNo( const std::string& serialNo );
    void setVersion( const std::string& version );
    void setIPAddress( const std::string& ipAddress );
    void setSubnetMask( const std::string& subNetmask );
    void setDefaultGateway( const std::string& defaultGateway );
    void setDhcpEnable( const bool dhcpEnable );    
private:
    class Internel;
    Internel *m_internel;    
};  

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_DEV_SEARCH_MESSAGE_H__

