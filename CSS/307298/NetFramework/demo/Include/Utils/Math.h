//
//  "$Id: ByteOrder.h 16503 2010-11-26 02:23:15Z wang_haifeng $"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __DAHUA_MATH_ORDER_H__
#define __DAHUA_MATH_ORDER_H__

#include "Infra/IntTypes.h"
#include "Defs.h"

namespace Dahua {
namespace Utils {

/// \param x 无符号整形变量值
/// \return 对数值整数
inline int log2i(uint32_t x)
{
	int r = 31;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

} // namespace Utils
} // namespace Dahua

#endif// __DAHUA_MATH_ORDER_H__

