//
//  "$Id$"
//
//  Copyright (c)1992-2012, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __DAHUA_TRIPLE_DES_H__
#define __DAHUA_TRIPLE_DES_H__

#include <stddef.h>	// for size_t
#include "Infra/IntTypes.h"
#include "Defs.h"


namespace Dahua {
namespace Utils {

/// Triple DES 算法处理类
class UTILS_API CTripleDes
{
	CTripleDes(CTripleDes const&);
	CTripleDes& operator=(CTripleDes const&);

public:
	///< 密钥数据大小
	enum {KeySize = 24};

	/// 算法处理的块字节数类型
	enum BlockType
	{
		block8,		///< 每次处理8字节
		block16,	///< 每次处理16字节
	};

	/// 构造函数
	/// \param [in] key 密钥数据指针
	/// \param [in] size 密钥数据有效长度，不超过 KeySize，如果为8个字节，则是des加密，否则是3des加密
	/// \param [in] block 3des加密时，算法每次处理的块字节数类型，iv的字节数也由此指定
	/// \param [in] iv 初始向量指针，长度必须为block对应的大小，如果为NULL，使用EBC模式，否则为CBC模式
	CTripleDes(uint8_t const* key, size_t size, BlockType block = block8, uint8_t const* iv = NULL);

	/// 析构函数
	~CTripleDes();

	///	加密
	/// \param [out] dst 加密后的数据缓冲指针
	///	\param [in] src 原始数据缓冲指针
	/// \param [in] size 数据长度，必须是 BlockSize 的整数倍，否则会截断
	/// \return 返回是否成功
	bool encrypt(uint8_t* dst, uint8_t const* src, size_t size);

	///	解密
	/// \param [out] dst 解密后的数据缓冲指针
	///	\param [in] src 密文数据缓冲指针
	/// \param [in] size 数据长度，必须是 BlockSize 的整数倍，否则会截断
	/// \return 返回是否成功
	bool decrypt(uint8_t* dst, uint8_t const* src, size_t size);

private:
	struct TripleDesInternal;
	TripleDesInternal* m_internal;

};


} // namespace Utils
} // namespace Dahua

#endif// __DAHUA_TRIPLE_DES_H__


