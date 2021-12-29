//  "$Id: Unicode.h 16503 2010-11-26 02:23:15Z wang_haifeng $"
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

/*
 * linux/fs/nls_cp936.c
 *
 * Charset cp936 translation tables.
 * This translation table was generated automatically, the
 * original table can be download from the Microsoft website.
 * (http://www.microsoft.com/typography/unicode/unicodecp.htm)
 */
#ifndef __DAHUA_UNICODE_H__
#define __DAHUA_UNICODE_H__

#include "Infra/IntTypes.h"
#include "Defs.h"


namespace Dahua {
namespace Utils {

/// \defgroup Unicode Unicode Convert
/// Unicode utf-8 GBK �ַ��������ʽת��
/// @{
//utf8�Ǳ䳤���룬����ռ3����Ӣ��ռ1���ֽ�
//unicode�ǵȳ�����Ӣ�Ķ���2��
//gb2312Ҳ�Ǳ䳤������ռ2����Ӣ��ռ1���ֽ�
//utf8�Ƕ��ռ����ֽڵĿ��ַ���2312��һ������ռ�������ֽ�,����utf8��gb2312�е���ĸ���ǵ��ֽڣ�
//������ַ�������ȷ�ϣ�����utf8ת������gb2312���ֽ����϶���֮ǰҪС��


/// unicode�ַ���תGB2312�����ַ���
/// \param [in] pUnicode	��ű���ǰ��unicode���ı�����
/// \param [in] nUnicLen	ת����unicode�ַ��ĸ�������
/// \param [in] pStr		�����GB2312��Ĵ�ŵ�λ��
/// \param [in] nStrLen		�����GB2312��Ļ�����������ֽ��������ַ����޹�
/// \return					����ʵ�ʱ���ת����Unicode�ַ�����
int UTILS_API UniTGb2312(uint16_t* pUnicode, int nUnicLen, char* pStr, int nStrLen);

/// GB2312�ַ���(����0��β����Ϊ��char*��ʾ���ֽ���)תUnicode�����ַ���(������0��β�������ɷ���ֵ��nLenTag����)
/// \param [in] pSource		���ת��ǰ��GB2312���ı�����,��Ҫ��'\0'��βΪֹ(��Ϊû�������������source����)
/// \param [in] pTag		ת����unicode�ַ��Ĵ洢��λ��,���������β��һ����'\0'
/// \param [in] nLenTag		�����Unicode��Ŀ��ַ������������
/// \return					���ر���ת���õ���Unicode�ַ���������'\0'
int UTILS_API Gb2312ToUni(char* pSource, uint16_t* pTag, int nLenTag);

/// GB2312�ַ���(����0��β����Ϊ��char*��ʾ���ֽ���)תUTF-8�����ַ���(������0��β��������nUtf8Len�ͷ���ֵ����)
/// \param [in] pSource		���ת��ǰ��GB2312���ı�����,��Ҫ��'\0'��βΪֹ(��Ϊû�������������source����)
/// \param [in] pUtf8		ת����Utf8�ֽڴ�ŵ�λ�ã����������β��һ����'\0'
/// \param [in] nUtf8Len	�����Utf8����ַ������������
/// \return					���ر���ת���õ���Utf8��ռ�õ��ַ���
int UTILS_API Gb2312TUtf8(char* pSource, uint8_t* pUtf8, int nUtf8Len);

/// UTF8�ַ���(��0��β����Ϊ��char*��ʾ���ֽ���)תGB2312�����ַ���(������0��β��������nStrLen��nUtf8Len����)
/// \param [in] pSource		���ת��ǰ��UTF8���ı�����,��Ҫ��'\0'��βΪֹ(��Ϊû�������������source����)
/// \param [in] pStr		ת����GB2312�ַ��Ĵ洢��λ��,���������β��һ����'\0'
/// \param [in] nUtf8Len	��Ҫת����UTF8���ֽڸ���
/// \param [in] nStrLen		�洢ת����GB2312�Ļ�������󳤶�
/// \return					���ر���ת���е�����ת����ʵ���ַ�����
int UTILS_API utf8TGb2312(unsigned char* putf8, char* pStr, int nUtf8Len, int nStrLen);

/// unicode�ַ�תGB2312�����ַ�
/// \param [in] uni			��ű���ǰ��unicode���ַ�
/// \param [in] out			�����GB2312��Ĵ�ŵ�ָ��λ��
/// \param [in] boundlen	�߽糤��λ��
/// \return					���ر�����unicode�ַ�ת�����GB2312���ֽڳ���
int UTILS_API uni2char(const uint16_t uni, uint8_t *out, int boundlen);

/// GB2312(�������ֽ���)���ַ�תUnicode������ַ�
/// \param [in] rawstring		���ת��ǰ��GB2312��һ���ַ���Ӧ��GB2312���ֽ��ַ���
/// \param [in] boundlen		�߽��ַ�����
/// \param [in/out] uni			�����unicode��Ĵ�ŵ�λ��
/// \return						����ת����Ҫʹ�õ�GB2312���ֽڸ���
int UTILS_API char2uni(const uint8_t *rawstring, int boundlen, uint16_t *uni);

/// utf8���ֽڶ�Ӧ���ַ�ת��unicode�Ŀ��ַ�
/// \param [in] s			���ֽڵ�utf8�ֽ�
/// \param [in] pwcs		ת����Ŀ��ַ��Ĵ��λ��
/// \param [in] n			��Ҫת����utf8���ֽڸ���
/// \return					���ر���ת���ɹ���UTF8���ֽڸ���
int UTILS_API utf8_mbtowc(uint16_t *p, const uint8_t *s, int n);

/// utf8���ֽڴ�ת��unicode�Ŀ��ַ���
/// \param [in] s			���ֽڵ�utf8�ֽ���
/// \param [in] pwcs		ת����Ŀ��ַ����Ĵ��λ��
/// \param [in] n			��Ҫת����utf8���ֽڸ���
/// \return					���ر���ת���ɹ���Unicode���ַ�����(Ҳ��pwcs��ŵ�Unicode���ַ�����)
int UTILS_API utf8_mbstowcs(uint16_t *pwcs, const uint8_t *s, int n);

/// utf8���ַ�ת��Ϊ���ֽ�
/// \param [in] s			���ת�����Utf8���ֽ�λ��
/// \param [in] wc			ת��ǰ�Ŀ��ַ�ֵ
/// \param [in] maxlen		���ת��ʹ�õ�utf8�ֽڵĳ���
/// \return					���ر���ת����ռ�õ��ֽ���(����'0')
int UTILS_API utf8_wctomb(uint8_t *s, uint16_t wc, int maxlen);

/// unicode�Ŀ��ַ���ת��Ϊutf8���ֽڴ�
/// \param [in] s			���ת�����Utf8���ֽ�λ��
/// \param [in] pwcs		ת��ǰ�Ŀ��ַ��ֽڴ�ŵ�λ��
/// \param [in] maxlen		�洢utf8�ֽڻ����������ֵ����
/// \return					���ر���ת����ռ�õ��ֽ���
int UTILS_API utf8_wcstombs(uint8_t *s, const uint16_t *pwcs, int maxlen);

//��utf8�ַ���ȡһ��Unicode�ַ������������Ӧ��utf8�ֽ���
/// \param [in] pch			���ֽڵ�utf8�ַ���
/// \param [in] pn			���ر���ת���ɹ������Ӧ��utf8�ֽ���
/// \return					ת����Ŀ��ַ�
uint16_t UTILS_API GetUnicodeFromUtf8(const char *pch, int *pn);

/// �� cp1252ת����utf8
/// \param [out] utf8buf utf-8�ַ�����ŵ�ַ
/// \param [in] utf8buflen utf-8�ַ�����ŵ�ַ�����������ֽڼǣ�
/// \param [in] cp1252buf cp1252�����ַ��ĵ�ַ
/// \param [in] cp1252buflen cp1252�����ַ��ĳ��ȣ����ֽڼǣ�
/// \return -1--ʧ�ܣ�0--�ɹ��� >0 ��ʾ���岻������utf8buflen�Ŀռ䲻����Ҫ�ѿռ����ӵ�����ֵָ����ֵ
int  UTILS_API cp1252_to_utf8(uint8_t *utf8buf, int utf8buflen, const uint8_t *cp1252buf, int cp1252buflen);

/// �� utf8ת����cp1252
/// \param [in] utf8buf utf-8�ַ�����ŵ�ַ
/// \param [in] utf8buflen utf-8�ַ�����ŵ�ַ�����������ֽڼǣ�
/// \param [out] cp1252buf cp1252�����ַ��ĵ�ַ
/// \param [in] cp1252buflen cp1252�����ַ��ĳ��ȣ����ֽڼǣ�
/// \return -1--ʧ�ܣ�0--�ɹ��� >0 ��ʾ���岻������utf8buflen�Ŀռ䲻����Ҫ�ѿռ����ӵ�����ֵָ����ֵ
int UTILS_API utf8_to_cp1252(const uint8_t *utf8buf, int utf8buflen, uint8_t *cp1252buf, int cp1252buflen);

/// \brief ����ASCII�ĵ��ֽڱ���ת��ΪUTF-8����
/// \param [in]     dst    UTF-8�ַ�����ŵ�ַ
/// \param [in/out] dstLen dst���ֽڳ���/ת����ʵ��ʹ�õ��ֽڳ���
/// \param [in]     src    ��ת���ַ���ַ
/// \param [in/out] srcLen src���ֽڳ���/ʵ�ʴ������Դ�ֽڳ���
/// \return -1:ʧ�� 0:�ɹ�
/// \note ��������ת��Ϊunicode��������������������ת��
/// \note cp874����iso8859-11ת��
int UTILS_API cp862_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API cp874_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API cp1250_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API cp1251_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API iso8859_1_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API iso8859_2_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API iso8859_3_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API iso8859_4_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API iso8859_5_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API iso8859_6_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API iso8859_7_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API iso8859_8_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API iso8859_9_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API iso8859_10_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API iso8859_13_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API iso8859_14_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API iso8859_15_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);
int UTILS_API iso8859_16_to_utf8(uint8_t *dst, int& dstLen, const uint8_t *src, int& srcLen);


/// @} end of group

} // namespace Utils
} // namespace Dahua

#endif // __DAHUA_UNICODE_H__

