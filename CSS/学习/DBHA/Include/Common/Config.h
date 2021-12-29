//
//  "$Id: Config.h  2013年11月13日  zhang_hailong 23919 $"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	1、一个CConfig类对应一个配置文件
//                  2、写配置时，先写到 file.bak 文件中，如成功，再尝试将file改名为file.prev，将file.bak改名为file
//                  3、加载配置时，如果读取file失败，将尝试读file.prev。如读取file.prev成功，将file.prev改名为ile
//                  4、暂不支持json值嵌套的场景，即某项的值本身又是一个json对象
//                  5、CConfig类内部已经考虑了线程安全，可以在多个线程中使用同一个CConfig对象。
//                     但是如果不同线程间，通过不同的CConfig类操作同一个文件，无法保证其正确性
//                  使用示例：
//                    CConfig  cfg("/etc/abc.conf");
//                    if (cfg.init()) {
//                        string  name;
//                        cfg.getString("name", name);
//                    }
//
//                  配置文件示例:
//                  {
//                      "city" : "Beijing",
//                      "mobile phone" : [ "215109234", "3015109234", "1620141938" ]
//                      "name" : "Jhon",
//                      "postcode" : 310052,
//                      "salary" : 3.14150,
//                      "street" : " Chaoyang Road "
//                  }
//	Revisions:		Year-Month-Day     SVN-Author         Modification
//					2013年11月13日	   zhang_hailong	  Create
//

#ifndef WIN32

#ifndef __INCLUDE_DAHUA_EFS_CONFIG_H__
#define __INCLUDE_DAHUA_EFS_CONFIG_H__

#include <string>
#include "Infra/Types.h"

namespace Dahua {
namespace EFS {

class CConfig {
public:
    // input:当isLoadingFromFile为true时，表示配置文件绝对路径,例如： /etc/test.conf。
    //       当isLoadingFromFile为false时，表示特定格式（如jason）的字符串。
    CConfig(const std::string& input, bool isLoadingFromFile);
    virtual ~CConfig();

    // 构造后必须先调用一次init，加载配置文件
    // 返回值： true，加载成功；
    //       false，加载配置文件失败，需查看日志。可能是文件不存在、json格式错误等
    bool  init();

    // 重新加载配置
    // keys 指定重新加载的配置项。
    // 返回值：成功时返回false，失败时返回true。
    bool reload( const std::vector<std::string>& keys );

    bool  getString(const std::string& key, std::string& val);
    bool  setString(const std::string& key, const std::string& val);
    
    // 如果key对应的元素为array格式，则将array的每一个元素都转化成字符串的形式，并都添加到vector中。
    // 适用于数组成员为一个复杂结构的类型
    // key 键值。
    // val 输出参数，转化后的字符串存于该结构中。
    // 返回值：成功时返回true，失败返回false。
    bool  getArray(const std::string& key, std::vector<std::string>& val);
    
    // 获取字符串数组
    bool  getStrArray(const std::string& key, std::vector<std::string>& val);
    bool  setStrArray(const std::string& key, const std::vector<std::string>& val);
    // node : 由于公司json库不支持long类型，所以不提供long类型操作接口
    bool  getInt(const std::string& key, int32_t& val);
    bool  setInt(const std::string& key, const int32_t val);
    bool  getUInt(const std::string& key, uint32_t& val);
    bool  setUInt(const std::string& key, const uint32_t val);
    bool  getDouble(const std::string& key, double& val);
    bool  setDouble(const std::string& key, const double val);
private:
    class Internal;
    class Internal* m_internal;
};

} /* namespace NetFramework */
} /* namespace Dahua */

#endif /* __DAHUA_EFS_CONFIG_H__ */
#endif
