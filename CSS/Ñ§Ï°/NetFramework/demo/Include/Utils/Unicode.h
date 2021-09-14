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

int UTILS_API UniTGb2312(uint16_t* pUnicode, int nUnicLen, char* pStr, int nStrLen);

int UTILS_API Gb2312ToUni(char* pSource, uint16_t* pTag, int nLenTag);

int UTILS_API Gb2312TUtf8(char* pSource, uint8_t* pUtf8, int nUtf8Len);

int UTILS_API utf8TGb2312(unsigned char* putf8, char* pStr, int nUtf8Len, int nStrLen);

int UTILS_API uni2char(const uint16_t uni, uint8_t *out, int boundlen);

int UTILS_API char2uni(const uint8_t *rawstring, int boundlen, uint16_t *uni);

int UTILS_API utf8_mbtowc(uint16_t *p, const uint8_t *s, int n);

int UTILS_API utf8_mbstowcs(uint16_t *pwcs, const uint8_t *s, int n);

int UTILS_API utf8_wctomb(uint8_t *s, uint16_t wc, int maxlen);

int UTILS_API utf8_wcstombs(uint8_t *s, const uint16_t *pwcs, int maxlen);

/// 从utf8字符串取一个Unicode字符，并传出其对应的utf8字节数
uint16_t UTILS_API GetUnicodeFromUtf8(const char *pch, int *pn);

/// @} end of group

} // namespace Utils
} // namespace Dahua

#endif // __DAHUA_UNICODE_H__






