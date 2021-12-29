//
//  "$Id$"
//
//  Copyright ( c )1992-2016, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	数据库HA框架公共函数
//	Revisions:		2017-01-12 #
//

#ifndef __DAHUA_VideoCloud_DBHA_COMMON_FUN_H__
#define __DAHUA_VideoCloud_DBHA_COMMON_FUN_H__

#include "HALog.h"
#include "Common/os.h"
#include <string>
#include <vector>

namespace Dahua {
namespace VideoCloud {
namespace DBHA{

typedef std::vector<std::string> StringVec;

static const char* const SyncState[] = {"syncInit", "syncing", "syncError", "recoverying", "recoveryEnd"};
static const char* const SlaveConnectState[] = {"connectInit", "connected", "connectError"};

class CommonFun
{
public:
    CommonFun();
    ~CommonFun();
	static bool executeScript(std::string scriptPath, std::string para);
	static int executeScript(std::string scriptPath);
	static bool getExecuteScriptValue(std::string scriptPath, std::string para, std::string& retValue);
	static int32_t strSplit( const std::string& src, char separator, StringVec& strs );
	static uint64_t getFileLen(std::string fileName);
	static bool setBinlogInfo(std::string binlogInfo, std::string& binlogName, int32_t& binlogPos);
};

} //DBHA
} //VideoCloud
} //Dahua

#endif // __DAHUA_VideoCloud_DBHA_COMMON_FUN_H__
