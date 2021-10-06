//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_DEV_SET_CONFIG_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_DEV_SET_CONFIG_MESSAGE_H__

#include "Infra/IntTypes.h"
#include <string>

namespace Dahua {
namespace EFS {     

class CDevSetConfigMessage
{
public:
    CDevSetConfigMessage( );
    ~CDevSetConfigMessage( );
    bool getMethod( std::string& method ) ;
    bool getMac( std::string& mac ) ;
    bool getUserName( std::string& userName );
    bool getPassword( std::string& password );
    bool getAuthorityType( std::string& authorityType );
    bool getIPAddress( std::string& ipAddress );
    bool getIpAddressOld( std::string& ipAddressOld );
    bool getSubnetMask( std::string& subnetMask );
    bool getDefaultGateway( std::string& defaultGateway ); 
private:
    class Internel;
    Internel *m_internel;     
};

class CResDevSetConfigMessage
{
public:
    CResDevSetConfigMessage( );
    ~CResDevSetConfigMessage( );

    void setMethod( const std::string& method );
	void setMac(const std::string& mac);
    void setResult( const bool result );
    void setCode( const int code );
    void setRealm( const std::string& realm );
    void setRandom( const std::string& random );
    void setEncryption( const std::string& encryption );
private:
    class Internel;
    Internel *m_internel;     
};  

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_DEV_SET_CONFIG_MESSAGE_H__

