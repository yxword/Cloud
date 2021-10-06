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

/// SHA-1 �㷨������
class UTILS_API CSha1
{
	CSha1(CSha1 const&);
	CSha1& operator=(CSha1 const&);

public:

	enum {Sha1HashSize = 20};	///< ���ɵ� HASH ���ݴ�С
	enum {Sha1HexLength = 40};	///< ���ɵ� HEX �ַ�������

	/// ���캯��
	CSha1();

	/// ��������
	~CSha1();

	///	׷��Ҫ�� SHA1 ժҪ�����ݣ��ýӿڿɵ��ö�Σ��ֱ�׷�����ݡ�
	///	\param [in] data ����ָ��
	/// \param [in] size ���ݳ���
	void update(uint8_t const* data, size_t size);

	///	���׷�����ݣ������� Sha1HashSize �ֽڵĽ������
	///	\param [out] hash �����������
	/// \param [in] len ��������ֽ���������С�� Sha1HashSize
	void final(uint8_t* hash, size_t len);

	///	���׷�����ݣ��������ת���� Sha1HexLength �ֽڵ��ַ���(16����Сд�ַ���)
	///	\param [out] str �ַ���������棬�����ֽڴ��� Sha1HashSize ʱ�������ִ�ĩβ�� '\0'
	/// \param [in] len �ַ��������ֽ���������С�� Sha1HashSize
	void hex(char* str, size_t len);

private:
	struct Sha1Internal;
	Sha1Internal* m_internal;

};


} // namespace Utils
} // namespace Dahua

#endif// __DAHUA_SHA1_H__


