//
//  "$Id$"
//
//  Copyright (c)1992-2011, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __DAHUA_MD5_H__
#define __DAHUA_MD5_H__

#include <stddef.h>	// for size_t
#include "Infra/IntTypes.h"
#include "Defs.h"


namespace Dahua {
namespace Utils {

struct Md5Internal;

/// MD5 算法处理类
class UTILS_API CMd5
{
	CMd5(CMd5 const&);
	CMd5& operator=(CMd5 const&);

public:
	/// 构造函数
	CMd5();

	/// 析构函数
	~CMd5();

	///	初始化
	void init();

	///	追加要做MD5摘要的数据，该接口可调用多次，分别追加数据。
	///	\param [in] data 数据指针
	/// \param [in] size 数据长度
	void update(uint8_t const* data, size_t size);

	///	生成16字节MD5摘要
	///	\param [out] digest 摘要输出缓存，不可小于16字节！
	void final(uint8_t* digest);

	///	生成32字节MD5摘要哈希(16进制小写字符串)
	///	\param [out] hash 摘要哈希输出缓存，不可小于32字节！
	void hex(char* hash);

private:
	Md5Internal* m_internal;

};


} // namespace Utils
} // namespace Dahua

#endif// __DAHUA_MD5_H__


