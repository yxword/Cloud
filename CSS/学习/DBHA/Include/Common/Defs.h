//
//  "$Id: Defs.h 16861 2010-12-03 07:41:03Z wang_haifeng $"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_PROTOCOL_DEFS_H__
#define __INCLUDE_DAHUA_EFS_PROTOCOL_DEFS_H__

#include <stdlib.h>
#include <assert.h>
#include "Log.h"

namespace Dahua{
namespace EFS{
	
#define UINT64_CAST(d) (unsigned long long)(d)      // 兼容64位和32位机打印 %llu,本宏仅限于打印时使用 
#define INT64_CAST(d) (long long)(d)      // 兼容64位和32位机打印 %ll,本宏仅限于打印时使用


#ifdef NDEBUG
#define ASSERT(expression,flag) do{if(!(expression)){Dahua::EFS::efsLogFatal("Assert fail:%s, file %s, function %s, line %d\n",\
	#expression,__FILE__,__FUNCTION__,__LINE__);if(flag) abort();}}while(0)
	
#define ASSERT_RET(expression, ret) do{if(!(expression)){Dahua::EFS::efsLogFatal("Assert fail:%s, file %s, function %s, line %d\n",\
	#expression,__FILE__,__FUNCTION__,__LINE__);return ret;}}while(0)

#define ASSERT_ABORT(expression) ASSERT(expression,true)
#define ASSERT_PRINT(expression) ASSERT(expression,false)
#define ASSERT_VOID(expression) ASSERT_RET(expression, )
#else

#define ASSERT(expression,flag) assert(expression)
#define ASSERT_RET(expression, ret) assert(expression)

#define ASSERT_ABORT(expression) assert(expression)
#define ASSERT_PRINT(expression) assert(expression)
#define ASSERT_VOID(expression) assert(expression)
#endif

}
}

#endif // __INCLUDE_DAHUA_EFS_PROTOCOL_DEFS_H__
