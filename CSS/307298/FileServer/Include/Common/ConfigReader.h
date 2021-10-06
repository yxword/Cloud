//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

//k=v格式的配置文件读取类,配置文件中支持'#'符号开头的注释

#ifndef __INCLUDE_DAHUA_EFS_CONFIG_READER_H__
#define __INCLUDE_DAHUA_EFS_CONFIG_READER_H__

#include "Infra/IntTypes.h"
#include <string>
#include <set>
#include <map>

namespace Dahua {
namespace EFS {

class CConfigReader
{
public:
	CConfigReader();
	~CConfigReader();
	//加载配置,需要在注册之后才可加载
	bool loadConfig(const char* fileName);
	//根据key去获取指定类型的value;一个key对应的value只有一种类型
	bool getStringValue(const std::string& key,std::string& outValue);
	bool getLongValue(const std::string& key,int64_t* outValue);
	bool getDoubleValue(const std::string&key,double* outValue);
	//注册key,并指定期望value类型;不允许key重复注册
	bool registerKeyByString(const std::string& key,const std::string& defaultValue="null");
	bool registerKeyByLong(const std::string& key,int64_t defaultValue=0);
	bool registerKeyByDouble(const std::string& key,double defaultValue=0);
private:
	struct Internal;
	struct Internal* m_internal;
};

} // namespace EFS
} // namespace Dahua
#endif //__INCLUDE_DAHUA_EFS_CONFIG_READER_H__

