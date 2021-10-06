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

/// Triple DES �㷨������
class UTILS_API CTripleDes
{
	CTripleDes(CTripleDes const&);
	CTripleDes& operator=(CTripleDes const&);

public:
	///< ��Կ���ݴ�С
	enum {KeySize = 24};

	/// �㷨����Ŀ��ֽ�������
	enum BlockType
	{
		block8,		///< ÿ�δ���8�ֽ�
		block16,	///< ÿ�δ���16�ֽ�
	};

	/// ���캯��
	/// \param [in] key ��Կ����ָ��
	/// \param [in] size ��Կ������Ч���ȣ������� KeySize�����Ϊ8���ֽڣ�����des���ܣ�������3des����
	/// \param [in] block 3des����ʱ���㷨ÿ�δ���Ŀ��ֽ������ͣ�iv���ֽ���Ҳ�ɴ�ָ��
	/// \param [in] iv ��ʼ����ָ�룬���ȱ���Ϊblock��Ӧ�Ĵ�С�����ΪNULL��ʹ��EBCģʽ������ΪCBCģʽ
	CTripleDes(uint8_t const* key, size_t size, BlockType block = block8, uint8_t const* iv = NULL);

	/// ��������
	~CTripleDes();

	///	����
	/// \param [out] dst ���ܺ�����ݻ���ָ��
	///	\param [in] src ԭʼ���ݻ���ָ��
	/// \param [in] size ���ݳ��ȣ������� BlockSize ���������������ض�
	/// \return �����Ƿ�ɹ�
	bool encrypt(uint8_t* dst, uint8_t const* src, size_t size);

	///	����
	/// \param [out] dst ���ܺ�����ݻ���ָ��
	///	\param [in] src �������ݻ���ָ��
	/// \param [in] size ���ݳ��ȣ������� BlockSize ���������������ض�
	/// \return �����Ƿ�ɹ�
	bool decrypt(uint8_t* dst, uint8_t const* src, size_t size);

private:
	struct TripleDesInternal;
	TripleDesInternal* m_internal;

};


} // namespace Utils
} // namespace Dahua

#endif// __DAHUA_TRIPLE_DES_H__


