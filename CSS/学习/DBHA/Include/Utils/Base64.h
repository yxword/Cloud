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

/// �����ű�������ݵĻ������ֽ���
int UTILS_API base64EncodeLen(int len);

/// Base64 ����
/// \param [in] dst ��ű������ı�����,dst����Ŀռ䳤�ȱ�����ڵ���base64EncodeLen������ı�������ݳ���,���򽫵��»��������
/// \param [in] src ����ǰ��Դ����
/// \param [in] len ����ǰ�������ֽ���
/// \return ���ر������ı��ֽ���, �������ַ���������
int UTILS_API base64Encode(char* dst, const char* src, int len);

/// �����Ž�������ݵĻ������ֽ���
int UTILS_API base64DecodeLen(const char* src);

/// Base64 ����
/// \param [in] dst �������ı�����,dst����Ŀռ䳤�ȱ�����ڵ���base64DecodeLen������Ľ�������ݳ���,���򽫵��»��������
/// \param [in] src ����ǰ��Դ����
/// \return ���ؽ����������ֽ���
int UTILS_API base64Decode(char* dst, const char* src);


} // namespace Utils
} // namespace Dahua

#endif// __DAHUA_BASE64_ORDER_H__


