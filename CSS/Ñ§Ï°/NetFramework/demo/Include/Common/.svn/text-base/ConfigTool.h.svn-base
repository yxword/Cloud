//
//  "$Id: DatanodeConfig.h  2013年11月14日  zhang_hailong 23919 $"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	能同时管理多个配置文件，每个配置文件要设置一个唯一的标志--domain,如“efs”，“datanode”等
//                  使用示例:
//                      Dahua::EFS::CMultipleConfigTool cfg;
//                      string domain = "mds";
//                      cfg.loadConfigFile("/etc/mds.conf", domain);
//                      string name;
//                      cfg.getString(domain,"name",name);
//	Revisions:		Year-Month-Day     SVN-Author         Modification
//					2013年11月14日	   zhang_hailong	  Create
//

#ifndef WIN32
#ifndef __INCLUDE_DAHUA_EFS_CONFIG_TOOL_H__
#define __INCLUDE_DAHUA_EFS_CONFIG_TOOL_H__

#include <string>
#include <vector>
#include "Infra/Types.h"

namespace Dahua {
namespace EFS {

class CConfigTool {
public:
    CConfigTool();
    virtual ~CConfigTool();

    // 通过（jason或其他格式的）字符串来直接加载，支持通过本工具，将字符串再次提取key/value。
    // input jason或其他格式的字符串。
    // domain 自定义的配置域，每个文件对应一个域，域名任意，只要不重复即可。例如： datanode, mds 等。
    // 返回值：成功时返回true，失败时返回false。
    bool loadConfig(const std::string& input, const std::string& domain);
    // ConfigTool中删除一个项配置，与loadConfig配套使用
    // domain 自定义的配置域。
    // 返回值：不存在domain域时返回false，成功删除时返回true。
    bool unloadConfig( const std::string& domain );	
    
    // file: 配置文件绝对路径
    // domain:自定义的配置域，每个文件对应一个域，域名任意，只要不重复即可。例如： datanode, mds 等
    bool  loadConfigFile(const std::string& file,const std::string& domain);


    bool  getString(const std::string& domain, const std::string& key, std::string& val);
    bool  setString(const std::string& domain, const std::string& key, const std::string& val);
    
    // 如果key对应的元素为array格式，则将array的每一个元素都转化成字符串的形式，并都添加到vector中。
    // 适用于数组成员为一个复杂结构的类型
    // key 键值。
    // val 输出参数，转化后的字符串存于该结构中。
    // 返回值：成功时返回true，失败返回false。
    bool getArray(const std::string& domain, const std::string& key, std::vector<std::string>& val);
    
    // 获取字符串数组成员
    bool  getStrArray(const std::string& domain, const std::string& key, std::vector<std::string>& val);
    bool  setStrArray(const std::string& domain, const std::string& key, const std::vector<std::string>& val);
    // node : 由于公司json库不支持long类型，所以不提供long类型操作接口
    bool  getInt(const std::string& domain, const std::string& key, int32_t& val);
    bool  setInt(const std::string& domain, const std::string& key, const int32_t val);
    bool  getDouble(const std::string& domain, const std::string& key, double& val);
    bool  setDouble(const std::string& domain, const std::string& key, const double val);
private:
    class Internal;
    class Internal* m_internal;
};

} /* namespace EFS */
} /* namespace Dahua */
#endif /* __DAHUA_EFS_DATANODECONFIG_H__ */
#endif
