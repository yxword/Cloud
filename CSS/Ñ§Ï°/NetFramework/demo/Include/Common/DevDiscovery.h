//
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                 
//
#ifndef __INCLUDE_DAHUA_EFS_DEV_DISCOVERY_H__
#define __INCLUDE_DAHUA_EFS_DEV_DISCOVERY_H__

#include "Infra/IntTypes.h"
#include "DevSearchMessage.h"
#include "DevSetConfigMessage.h"
#include "Infra/Function.h"
#include <string>

namespace Dahua {
namespace EFS {

class CDevDiscovery
{
public:
	//搜索设备请求回调函数
    typedef Infra::TFunction2<bool, CDevSearchMessage&, CResDevSearchMessage& > SearchCallBack;
	//校验修改IP的请求包信息
    typedef Infra::TFunction2<bool, CDevSetConfigMessage&, CResDevSetConfigMessage& > SetConfigCallBack;
	//修改IP及重启网卡（该回调函数的参数bool无意义）
    typedef Infra::TFunction1<void, bool > NetmanageCallBack;
public:
    CDevDiscovery( );
    virtual ~CDevDiscovery( );
    void setIP( const std::string& addr );
    void setSearchCallBack( const SearchCallBack& proc );
    void setsetConfigCallBack( const SetConfigCallBack& proc );
    void setNetmanageCallBack( const NetmanageCallBack& proc );

    bool init();
    int run();
private:
    class Internel;
    Internel *m_internel;    
};

}
}
#endif //__INCLUDE_DAHUA_EFS_DEV_DISCOVERY_H__

