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

/// \ǰ������
class AesInternal;

/// \brief AES�ӽ���
/// \note �ӽ��ܱ���ʹ�ò�ͬ����(ECB����)
/// -     ��ͬ�Ŀ���ʹ�ò�ͬ����(����������)
class UTILS_API CAes
{
public:
	typedef enum
	{
		MODE_ECB    = 0, ///< ���뱾ģʽ,Դ���ݳ��ȱ�����16��������(����ȫ)
		MODE_CBC    = 1, ///< ���������ģʽ,Դ���ݳ��ȱ�����16��������(�Ƽ�)
		MODE_CFB8   = 2, ///< ���뷴��ģʽ
		MODE_CFB128 = 3, ///< ���뷴��ģʽ
		MODE_CTR    = 4  ///< ������ģʽ
	}EncMode;

	typedef enum
	{
		BITS_128 = 128,
		BITS_192 = 192,
		BITS_256 = 256
	}KeyBits;

	/// ���캯��
	/// \param [in] mode     ����ģʽ
	/// \param [in] key      ������Կ
	/// \param [in] keybits  ��Կλ��
	/// \param [in] iv       ��ʼ������,������16 bytes
	/// \note ��ʹ��iv�ļ����㷨iv����ΪNULL,Ĭ��ʹ��0���;
	/// -     iv��CTRģʽʱ��ʾnonce_counter
	CAes(EncMode mode, const uint8_t* key, KeyBits keybits, const uint8_t* iv = NULL);

	/// ��������
	~CAes();

	///	����
	/// \param [out] dst ���ܺ�����ݻ���ָ��
	///	\param [in]  src ԭʼ���ݻ���ָ��
	/// \param [in]  size ���ݳ���
	/// \return �����Ƿ�ɹ�
	bool encrypt(uint8_t* dst, const uint8_t* src, size_t size);

	///	����
	/// \param [out] dst ���ܺ�����ݻ���ָ��
	///	\param [in]  src �������ݻ���ָ��
	/// \param [in]  size ���ݳ���
	/// \return �����Ƿ�ɹ�
	bool decrypt(uint8_t* dst, const uint8_t* src, size_t size);
private:
	AesInternal* m_internal;
};

} ///< end of Utils
} ///< end of Dahua

#endif ///< end of __DAHUA_UTILS_AES_H__
