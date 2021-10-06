//
//  "$Id$"
//
//  Copyright (c)1992-2011, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_ERROR_H__
#define __INCLUDE_DAHUA_EFS_ERROR_H__

namespace Dahua{
namespace EFS{

class CError{
public:
	//设置当前线程最后一次出现的错误码
	static void setLastError(int errcode);
	//获取当前线程最后一次出现的错误码
	static int getLastError();
	// 错误码到错误字符串的映射关系库内部完成，不提供接口
	// 根据错误码获取错误字符串。
	static const char* getErrMsg(int errcode);
};

} //namespace Dahua
} //EFS

#endif //__INCLUDE_DAHUA_EFS_ERROR_H__
