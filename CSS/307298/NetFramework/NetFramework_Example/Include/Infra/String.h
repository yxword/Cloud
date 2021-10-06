//
//  "$Id$"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __DAHUA_INFRA_STRING_H__
#define __DAHUA_INFRA_STRING_H__

#include "Defs.h"
#include "Detail/flex_string.h"


////////////////////////////////////////////////////////////////////////////////
// 与 std::basic_string 接口完全兼容的字符串处理类

namespace Dahua {
namespace Infra {

typedef flex_string<
	char,
	std::char_traits<char>,
	std::allocator<char>,
	SmallStringOpt<AllocatorStringStorage<char, std::allocator<char> >, 31>
> CString;

typedef flex_string<
	wchar_t,
	std::char_traits<wchar_t>,
	std::allocator<wchar_t>,
	SmallStringOpt<AllocatorStringStorage<wchar_t, std::allocator<wchar_t> >, 31>
> CWString;

} // namespace Infra
} // namespace Dahua


#endif// __DAHUA_INFRA_STRING_H__


