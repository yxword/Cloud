//
//  "$Id: Aes.h 433497 2017-03-20 06:58:28Z 20429 $"
//
//  Copyright (c)1992-2012, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __DAHUA_UTILS_AES_H__
#define __DAHUA_UTILS_AES_H__

#include <stddef.h>
#include "Infra/IntTypes.h"
#include "Defs.h"

namespace Dahua {
namespace Utils {

/// \前置声明
class AesInternal;

/// \brief AES加解密
/// \note 加解密必须使用不同对象(ECB例外)
/// -     不同的块需使用不同对象(流加密例外)
class UTILS_API CAes
{
public:
	typedef enum
	{
		MODE_ECB    = 0, ///< 电码本模式,源数据长度必须是16的整数倍(不安全)
		MODE_CBC    = 1, ///< 密码分组链模式,源数据长度必须是16的整数倍(推荐)
		MODE_CFB8   = 2, ///< 密码反馈模式
		MODE_CFB128 = 3, ///< 密码反馈模式
		MODE_CTR    = 4  ///< 计算器模式
	}EncMode;

	typedef enum
	{
		BITS_128 = 128,
		BITS_192 = 192,
		BITS_256 = 256
	}KeyBits;

	/// 构造函数
	/// \param [in] mode     加密模式
	/// \param [in] key      加密密钥
	/// \param [in] keybits  密钥位数
	/// \param [in] iv       初始化向量,必须是16 bytes
	/// \note 若使用iv的加密算法iv传入为NULL,默认使用0填充;
	/// -     iv在CTR模式时表示nonce_counter
	CAes(EncMode mode, const uint8_t* key, KeyBits keybits, const uint8_t* iv = NULL);

	/// 析构函数
	~CAes();

	///	加密
	/// \param [out] dst 加密后的数据缓冲指针
	///	\param [in]  src 原始数据缓冲指针
	/// \param [in]  size 数据长度
	/// \return 返回是否成功
	bool encrypt(uint8_t* dst, const uint8_t* src, size_t size);

	///	解密
	/// \param [out] dst 解密后的数据缓冲指针
	///	\param [in]  src 密文数据缓冲指针
	/// \param [in]  size 数据长度
	/// \return 返回是否成功
	bool decrypt(uint8_t* dst, const uint8_t* src, size_t size);
private:
	AesInternal* m_internal;
};

} ///< end of Utils
} ///< end of Dahua

#endif ///< end of __DAHUA_UTILS_AES_H__
