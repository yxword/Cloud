//
//  "$Id$"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __DAHUA_BASE64_ORDER_H__
#define __DAHUA_BASE64_ORDER_H__

#include "Defs.h"

namespace Dahua {
namespace Utils {

/// 计算存放编码后数据的缓冲区字节数
int UTILS_API base64EncodeLen(int len);

/// Base64 编码
/// \param [in] dst 存放编码后的文本缓存,dst分配的空间长度必须大于等于base64EncodeLen计算出的编码后数据长度,否则将导致缓冲区溢出
/// \param [in] src 编码前的源数据
/// \param [in] len 编码前的数据字节数
/// \return 返回编码后的文本字节数, 不包含字符串结束符
int UTILS_API base64Encode(char* dst, const char* src, int len);

/// 计算存放解码后数据的缓冲区字节数
int UTILS_API base64DecodeLen(const char* src);

/// Base64 解码
/// \param [in] dst 解码后的文本缓存,dst分配的空间长度必须大于等于base64DecodeLen计算出的解码后数据长度,否则将导致缓冲区溢出
/// \param [in] src 解码前的源数据
/// \return 返回解码后的数据字节数
int UTILS_API base64Decode(char* dst, const char* src);


} // namespace Utils
} // namespace Dahua

#endif// __DAHUA_BASE64_ORDER_H__


