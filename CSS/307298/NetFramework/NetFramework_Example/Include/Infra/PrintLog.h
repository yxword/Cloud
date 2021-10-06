//
//  "$Id$"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INFRA3_PRINT_LOG_H__
#define __INFRA3_PRINT_LOG_H__

#include <stdio.h>
#include "Defs.h"
#include "IntTypes.h"
#include "Function.h"


/// \defgroup PrintLog PrintLog
/// 打印信息记录等等。
/// 打印信息分trace, debug, info, warn, error, fatal共6个级别，记录的形式可以是
/// 直接打印到标准输出，也可以记录到文件或者发送到远程日志服务器，目前只支持
/// 第一种方式。每行打印信息前自动加上了时间和打印类型的字符串，比如"12:54:12|info"
/// @{

namespace Dahua{
namespace Infra{


/// 打印输出回调函数类型
/// 参数为要打印的字符串
typedef TFunction1<void, char const*> LogPrinterProc;

/// 设置打印的输出回调函数
/// \param [in] printer 输出回调函数, 为空时设置打印输出到标准输出设备
/// \return 成功返回0, 失败返回-1
int INFRA_API setLogPrinter(LogPrinterProc printer);

/// 设置打印的级别，高于该级别的打印不能输出
void INFRA_API setPrintLogLevel(int level);

/// 打印调用库名，记录调试信息，级别nLevel，不直接调用
/// \return 返回打印的字节数
int INFRA_API logLibName(int nLevel, const char* libName, const char* fmt, ...);

/// 记录调试信息，级别6，不直接调用，用于实现debgf宏
/// \return 返回打印的字节数
int INFRA_API logDebug(const char* fmt, ...);

/// 记录跟踪信息，级别5，不直接调用，用于实现tracef宏
/// \return 返回打印的字节数
int INFRA_API logTrace(const char* fmt, ...);

/// 记录调试信息，级别4，不直接调用，用于实现infof宏
/// \return 返回打印的字节数
int INFRA_API logInfo(const char* fmt, ...);

/// 记录警告信息，级别3，不直接调用，用于实现warnf宏
/// \return 返回打印的字节数
int INFRA_API logWarn(const char* fmt, ...);

/// 记录错误信息，级别2，不直接调用，用于实现errorf宏
/// \return 返回打印的字节数
int INFRA_API logError(const char* fmt, ...);

/// 记录致命错误信息，级别1，不直接调用，用于实现fatalf宏
/// \return 返回打印的字节数
int INFRA_API logFatal(const char* fmt, ...);

/// 以16进制和字符对应的方式打印内存数据
/// \param pdat 数据缓冲指针
/// \param length 数据缓冲长度
void INFRA_API dumpHex(uint8_t* pdat, size_t length = 512);


} // namespace Infra
} // namespace Dahua

//////////////////////////////////////////////////////////////////////////
// print message, controlled by messsage level

#undef debugf
#undef tracef
#undef infof
#undef warnf
#undef errorf
#undef fatalf
#undef tracepoint

#ifdef __GNUC__ // perform complete xxxxf macro replacement
#	ifdef LOG_LIB_NAME
#		define debugf(format, ...) logLibName(6, LOG_LIB_NAME, format, ## __VA_ARGS__)
#		define tracef(format, ...) logLibName(5, LOG_LIB_NAME, format, ## __VA_ARGS__)
#		define infof(format, ...) logLibName(4, LOG_LIB_NAME, format, ## __VA_ARGS__)
#		define warnf(format, ...) logLibName(3, LOG_LIB_NAME, format, ## __VA_ARGS__)
#		define errorf(format, ...) logLibName(2, LOG_LIB_NAME, format, ## __VA_ARGS__)
#		define fatalf(format, ...) logLibName(1, LOG_LIB_NAME, format, ## __VA_ARGS__)
#	else
#		define debugf(format, ...) logDebug(format, ## __VA_ARGS__)
#		define tracef(format, ...) logTrace(format, ## __VA_ARGS__)
#		define infof(format, ...) logInfo(format, ## __VA_ARGS__)
#		define warnf(format, ...) logWarn(format, ## __VA_ARGS__)
#		define errorf(format, ...) logError(format, ## __VA_ARGS__)
#		define fatalf(format, ...) logFatal(format, ## __VA_ARGS__)
#	endif
#else
#	define debugf logDebug
#	define tracef logTrace
#	define infof logInfo
#	define warnf logWarn
#	define errorf logError
#	define fatalf logFatal
#endif

#define tracepoint()  logDebug("tracepoint: %s, %d.\n",__FILE__,__LINE__)

/// @} end of group

#endif //__INFRA_PRINT_LOG_H__

