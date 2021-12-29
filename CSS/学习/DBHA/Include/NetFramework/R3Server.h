//
//  "$Id: SockAddr.h 26645 2011-05-31 00:35:01Z wang_haifeng $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2011-12-30		xu_ping	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_R3SERVER_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_R3SERVER_H__

#include "NDefs.h"

namespace Dahua {
namespace NetFramework {
		
// 启动服务器
// 返回值: 0 - success, -1 - fail
// 参数: port, 指定的监听端口; prompt, 控制台提示符; telwin, 是否在Windows下自动打开telnet窗口
int NETFRAMEWORK_API R3ServerStart(unsigned short port = 42323, const char *prompt = 0, bool telwin = true);

// 退出服务器
int NETFRAMEWORK_API R3ServerExit();

// 在R3控制台上打印
void NETFRAMEWORK_API R3Printf(const char *fmt, ...);

// 注册函数func，使用名字name，描述des
int NETFRAMEWORK_API R3Register(void *func, const char *name, const char *des = "");

/************************************************************************/
/* define cmd                                                           */
/************************************************************************/
struct TR3AutoReg
{
	TR3AutoReg(void *func, const char *name, const char *des)
	{
		R3Register(func, name, des);
	}
};

#define def_cmd(name)		static void _r3_cmd_##name
#define reg_cmd(name, des)	static TR3AutoReg _r3_auto_reg_##name((void *)_r3_cmd_##name, #name, des)

} //NetFramework
} //Dahua
	
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_R3SERVER_H__
