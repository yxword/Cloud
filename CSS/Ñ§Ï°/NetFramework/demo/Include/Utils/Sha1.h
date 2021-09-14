//
//  "$Id$"
//
//  Copyright (c)1992-2012, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __DAHUA_SHA1_H__
#define __DAHUA_SHA1_H__

#include <stddef.h>	// for size_t
#include "Infra/IntTypes.h"
#include "Defs.h"


namespace Dahua {
namespace Utils {

/// SHA-1 算法处理类
class UTILS_API CSha1
{
	CSha1(CSha1 const&);
	CSha1& operator=(CSha1 const&);

public:

	enum {Sha1HashSize = 20};	///< 生成的 HASH 数据大小
	enum {Sha1HexLength = 40};	///< 生成的 HEX 字符串长度

	/// 构造函数
	CSha1();

	/// 析构函数
	~CSha1();

	///	追加要做 SHA1 摘要的数据，该接口可调用多次，分别追加数据。
	///	\param [in] data 数据指针
	/// \param [in] size 数据长度
	void update(uint8_t const* data, size_t size);

	///	完成追加数据，并生成 Sha1HashSize 字节的结果数据
	///	\param [out] hash 数据输出缓存
	/// \param [in] len 输出缓存字节数，不可小于 Sha1HashSize
	void final(uint8_t* hash, size_t len);

	///	完成追加数据，并将结果转换成 Sha1HexLength 字节的字符串(16进制小写字符串)
	///	\param [out] str 字符串输出缓存，缓存字节大于 Sha1HashSize 时，会在字串末尾加 '\0'
	/// \param [in] len 字符串缓存字节数，不可小于 Sha1HashSize
	void hex(char* str, size_t len);

private:
	struct Sha1Internal;
	Sha1Internal* m_internal;

};


} // namespace Utils
} // namespace Dahua

#endif// __DAHUA_SHA1_H__


