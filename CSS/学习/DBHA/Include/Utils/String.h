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


/// ���ݲ�ͬƽ̨�µĺ���
typedef int (*snprintfFunc)(char *buffer, size_t count, const char *format, ... );
extern snprintfFunc UTILS_API snprintf;

/// ���ݲ�ͬƽ̨�µĺ���
typedef int (*stricmpFunc)(const char* str1, const char* str2);
extern stricmpFunc UTILS_API stricmp;

///	��ȫ���ַ�������
///	\param [out] dest:Ŀ��buffer,���Ŀ��bufferû��'\0'�������������һ���ַ�Ϊ'\0',����0
///	\param [in]	dstBufLen,Ŀ��buffer�ռ��С,�ú������д��dstBufLen-1���ַ���������д���ַ��������'\0'�ַ�
///	\param [in] src:
///	\param [in] srcCopyCount: ����src�ĳ���
///		ִ����ɺ������copy����ô��Destһ��������0����
///	\return ���صĽ���Ǵ�src copy��dest�е��ַ���Ŀ
UTILS_API size_t strncat(char* dest, size_t dstBufLen, const char* src, const size_t srcCopyCount);

///	��ȫ���ַ���copy����
///	\param [out] dst,Ŀ��buffer
///	\param [in]	dstBufLen,Ŀ��buffer�ռ��С,�ú������д��dstBufLen-1���ַ���������д���ַ��������'\0'�ַ�
///	\param [in]	src,Դbuffer
///	\param [in] srcCopyCount
///	\return Ҫcopy���ַ�����,��dstBufLen-1�ռ����������£����copy���ַ���ĿΪsrcCopyCount,�����ں������'\0'�ַ�
UTILS_API size_t strncpy(char* dst, size_t dstBufLen, const char* src, size_t srcCopyCount);

///	��ǿ��snprintf����֤'\0'������ʵ��д�볤�ȣ�
///	����֧�� len += snprintf_x( buf + len, maxlen - len, "xxxx", ... ); ������д��
///	��ʵ��buffer����ʱ����֤\'0'������maxlen - 1����ԭ��snprintf��VC�᷵��-1�Ҳ���֤'\0'��gcc�᷵�ؼ���buffer�㹻ʱ��д�볤�ȣ�
///	��������maxlen-1ʱ�޷����ֳ��ȸոպû��ǳ����ˣ����Լ򻯶������������߶�������������
///	Ҳ��������Ҫ�޳����ұ�֤'\0'ʱ���ַ���������ȡ��strncpy����ע��ԭ��strncpy����֤'\0'��
///	�� strncpy( dst, src, siz - 1 ); dst[size - 1] = '\0'; �൱�� snprintf( dst, siz, "%s", src );
///	\param [out] buf �������
///	\param [in] maxlen �����������ֽ���
///	\param [in] fmt ��ʽ�ַ���
///	\return ����ʵ��д�볤��
UTILS_API int snprintf_x(char* buf, int maxlen, const char* fmt, ... );

/// IP ��ַת��Ϊ�ַ���
/// \param [in] ip IP��ַ
/// \param [in] buf ת������ַ�����ַ
/// \param [in] len ת������ַ������泤��
/// \return ����ֵͬbuf
UTILS_API char* ipToString(IpAddress ip, char* buf, size_t len);

/// �ַ���ת��Ϊ IP ��ַ
/// \param [in] str Ҫת�����ַ���
/// \return ����ֵIP��ַ
UTILS_API IpAddress stringToIp(char const* str);


} // namespace Utils
} // namespace Dahua

#endif// __INFRA__BYTE_ORDER_H__


