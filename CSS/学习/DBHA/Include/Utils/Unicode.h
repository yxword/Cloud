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
/// Unicode utf-8 GBK 字符集编码格式转换
/// @{
//utf8是变长编码，中文占3个，英文占1个字节
//unicode是等长，中英文都是2个
//gb2312也是变长，中文占2个，英文占1个字节
//utf8是多个占多个字节的宽字符，2312是一个汉字占用两个字节,由于utf8和gb2312中的字母都是单字节，
//所以最长字符并不好确认，不过utf8转出来的gb2312的字节数肯定比之前要小。


/// unicode字符串转GB2312编码字符串
/// \param [in] pUnicode	存放编码前的unicode的文本缓存
/// \param [in] nUnicLen	转换的unicode字符的个数长度
/// \param [in] pStr		编码成GB2312后的存放的位置
/// \param [in] nStrLen		编码成GB2312后的缓冲区的最大字节数，与字符串无关
/// \return					返回实际编码转换的Unicode字符个数
int UTILS_API UniTGb2312(uint16_t* pUnicode, int nUnicLen, char* pStr, int nStrLen);

/// GB2312字符串(须以0结尾，因为是char*表示的字节数)转Unicode编码字符串(并不以0结尾，长度由返回值和nLenTag限制)
/// \param [in] pSource		存放转换前的GB2312的文本缓存,需要以'\0'结尾为止(因为没有输入参数标明source长度)
/// \param [in] pTag		转换的unicode字符的存储的位置,结果中最后结尾不一定是'\0'
/// \param [in] nLenTag		编码成Unicode后的宽字符个数最大限制
/// \return					返回编码转换得到的Unicode字符数，不含'\0'
int UTILS_API Gb2312ToUni(char* pSource, uint16_t* pTag, int nLenTag);

/// GB2312字符串(须以0结尾，因为是char*表示的字节数)转UTF-8编码字符串(并不以0结尾，长度由nUtf8Len和返回值限制)
/// \param [in] pSource		存放转换前的GB2312的文本缓存,需要以'\0'结尾为止(因为没有输入参数标明source长度)
/// \param [in] pUtf8		转换的Utf8字节存放的位置，结果中最后结尾不一定是'\0'
/// \param [in] nUtf8Len	编码成Utf8后的字符个数最大限制
/// \return					返回编码转换得到的Utf8所占用的字符数
int UTILS_API Gb2312TUtf8(char* pSource, uint8_t* pUtf8, int nUtf8Len);

/// UTF8字符串(以0结尾，因为是char*表示的字节数)转GB2312编码字符串(并不以0结尾，长度由nStrLen和nUtf8Len限制)
/// \param [in] pSource		存放转换前的UTF8的文本缓存,需要以'\0'结尾为止(因为没有输入参数标明source长度)
/// \param [in] pStr		转换的GB2312字符的存储的位置,结果中最后结尾不一定是'\0'
/// \param [in] nUtf8Len	需要转换的UTF8的字节个数
/// \param [in] nStrLen		存储转换成GB2312的缓冲区最大长度
/// \return					返回编码转换中的最终转换的实际字符个数
int UTILS_API utf8TGb2312(unsigned char* putf8, char* pStr, int nUtf8Len, int nStrLen);

/// unicode字符转GB2312编码字符
/// \param [in] uni			存放编码前的unicode的字符
/// \param [in] out			编码成GB2312后的存放的指针位置
/// \param [in] boundlen	边界长度位置
/// \return					返回编码后该unicode字符转换后的GB2312的字节长度
int UTILS_API uni2char(const uint16_t uni, uint8_t *out, int boundlen);

/// GB2312(不定长字节数)的字符转Unicode编码宽字符
/// \param [in] rawstring		存放转换前的GB2312的一个字符对应的GB2312的字节字符串
/// \param [in] boundlen		边界字符长度
/// \param [in/out] uni			编码成unicode后的存放的位置
/// \return						返回转换需要使用的GB2312的字节个数
int UTILS_API char2uni(const uint8_t *rawstring, int boundlen, uint16_t *uni);

/// utf8多字节对应的字符转换unicode的宽字符
/// \param [in] s			多字节的utf8字节
/// \param [in] pwcs		转换后的宽字符的存放位置
/// \param [in] n			需要转换的utf8个字节个数
/// \return					返回编码转换成功的UTF8的字节个数
int UTILS_API utf8_mbtowc(uint16_t *p, const uint8_t *s, int n);

/// utf8多字节串转换unicode的宽字符串
/// \param [in] s			多字节的utf8字节数
/// \param [in] pwcs		转换后的宽字符串的存放位置
/// \param [in] n			需要转换的utf8的字节个数
/// \return					返回编码转换成功的Unicode的字符个数(也即pwcs存放的Unicode宽字符个数)
int UTILS_API utf8_mbstowcs(uint16_t *pwcs, const uint8_t *s, int n);

/// utf8宽字符转换为多字节
/// \param [in] s			存放转换后的Utf8的字节位置
/// \param [in] wc			转换前的宽字符值
/// \param [in] maxlen		最多转换使用的utf8字节的长度
/// \return					返回编码转换后占用的字节数(不含'0')
int UTILS_API utf8_wctomb(uint8_t *s, uint16_t wc, int maxlen);

/// unicode的宽字符串转换为utf8多字节串
/// \param [in] s			存放转换后的Utf8的字节位置
/// \param [in] pwcs		转换前的宽字符字节存放的位置
/// \param [in] maxlen		存储utf8字节缓冲区的最大值长度
/// \return					返回编码转换后占用的字节数
int UTILS_API utf8_wcstombs(uint8_t *s, const uint16_t *pwcs, int maxlen);

//从utf8字符串取一个Unicode字符，并传出其对应的utf8字节数
/// \param [in] pch			多字节的utf8字符串
/// \param [in] pn			返回编码转换成功的其对应的utf8字节数
/// \return					转换后的宽字符
uint16_t UTILS_API GetUnicodeFromUtf8(const char *pch, int *pn);

/// 将 cp1252转换成utf8
/// \param [out] utf8buf utf-8字符待存放地址
/// \param [in] utf8buflen utf-8字符待存放地址的容量（以字节记）
/// \param [in] cp1252buf cp1252编码字符的地址
/// \param [in] cp1252buflen cp1252编码字符的长度（以字节记）
/// \return -1--失败，0--成功， >0 表示缓冲不够，即utf8buflen的空间不够，要把空间增加到返回值指定的值
int  UTILS_API cp1252_to_utf8(uint8_t *utf8buf, int utf8buflen, const uint8_t *cp1252buf, int cp1252buflen);

/// 将 utf8转换成cp1252
/// \param [in] utf8buf utf-8字符待存放地址
/// \param [in] utf8buflen utf-8字符待存放地址的容量（以字节记）
/// \param [out] cp1252buf cp1252编码字符的地址
/// \param [in] cp1252buflen cp1252编码字符的长度（以字节记）
/// \return -1--失败，0--成功， >0 表示缓冲不够，即utf8buflen的空间不够，要把空间增加到返回值指定的值
int UTILS_API utf8_to_cp1252(const uint8_t *utf8buf, int utf8buflen, uint8_t *cp1252buf, int cp1252buflen);

/// \brief 基于ASCII的单字节编码转换为UTF-8编码
/// \param [in]     dst    UTF-8字符待存放地址
/// \param [in/out] dstLen dst的字节长度/转换后实际使用的字节长度
/// \param [in]     src    待转换字符地址
/// \param [in/out] srcLen src的字节长度/实际处理过的源字节长度
/// \return -1:失败 0:成功
/// \note 遇到不可转换为unicode的情况会跳过继续后面的转换
/// \note cp874兼容iso8859-11转换
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

