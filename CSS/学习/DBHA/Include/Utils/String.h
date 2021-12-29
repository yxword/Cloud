//
//  "$Id$"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __UTILS_STRING_H__
#define __UTILS_STRING_H__

#include <stdio.h>
#include <string.h>
#include "Infra/Types.h"
#include "Defs.h"


namespace Dahua {
namespace Utils {


/// 兼容不同平台下的函数
typedef int (*snprintfFunc)(char *buffer, size_t count, const char *format, ... );
extern snprintfFunc UTILS_API snprintf;

/// 兼容不同平台下的函数
typedef int (*stricmpFunc)(const char* str1, const char* str2);
extern stricmpFunc UTILS_API stricmp;

///	安全的字符串连接
///	\param [out] dest:目标buffer,如果目标buffer没有'\0'结束，会设最后一个字符为'\0',返回0
///	\param [in]	dstBufLen,目标buffer空间大小,该函数最多写入dstBufLen-1个字符，并且在写入字符后面添加'\0'字符
///	\param [in] src:
///	\param [in] srcCopyCount: 拷贝src的长度
///		执行完成后，如果有copy，那么，Dest一定可以以0结束
///	\return 返回的结果是从src copy到dest中的字符数目
UTILS_API size_t strncat(char* dest, size_t dstBufLen, const char* src, const size_t srcCopyCount);

///	安全的字符串copy函数
///	\param [out] dst,目标buffer
///	\param [in]	dstBufLen,目标buffer空间大小,该函数最多写入dstBufLen-1个字符，并且在写入字符后面添加'\0'字符
///	\param [in]	src,源buffer
///	\param [in] srcCopyCount
///	\return 要copy的字符数码,在dstBufLen-1空间允许的情况下，最多copy的字符数目为srcCopyCount,并且在后面添加'\0'字符
UTILS_API size_t strncpy(char* dst, size_t dstBufLen, const char* src, size_t srcCopyCount);

///	增强的snprintf，保证'\0'，返回实际写入长度，
///	方便支持 len += snprintf_x( buf + len, maxlen - len, "xxxx", ... ); 的连续写法
///	当实际buffer不够时，保证\'0'，返回maxlen - 1，（原版snprintf，VC会返回-1且不保证'\0'，gcc会返回假设buffer足够时的写入长度）
///	（但返回maxlen-1时无法区分长度刚刚好还是出错了，可以简化都当出错处理，或者都当不出错不处理）
///	也可用于需要限长度且保证'\0'时的字符串拷贝，取代strncpy，（注意原版strncpy不保证'\0'）
///	即 strncpy( dst, src, siz - 1 ); dst[size - 1] = '\0'; 相当于 snprintf( dst, siz, "%s", src );
///	\param [out] buf 输出缓存
///	\param [in] maxlen 输出缓存最大字节数
///	\param [in] fmt 格式字符串
///	\return 返回实际写入长度
UTILS_API int snprintf_x(char* buf, int maxlen, const char* fmt, ... );

/// IP 地址转化为字符串
/// \param [in] ip IP地址
/// \param [in] buf 转换后的字符串地址
/// \param [in] len 转换后的字符串缓存长度
/// \return 返回值同buf
UTILS_API char* ipToString(IpAddress ip, char* buf, size_t len);

/// 字符串转化为 IP 地址
/// \param [in] str 要转换的字符串
/// \return 返回值IP地址
UTILS_API IpAddress stringToIp(char const* str);


} // namespace Utils
} // namespace Dahua

#endif// __INFRA__BYTE_ORDER_H__


