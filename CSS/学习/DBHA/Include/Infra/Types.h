//
//  "$Id: Types.h 16720 2010-12-01 09:51:53Z wang_haifeng $"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __DAHUA3_TYPES_H__
#define __DAHUA3_TYPES_H__


#include "Infra/IntTypes.h"

////////////////////////////////////////////////////////////////////////////////

namespace Dahua {

/// 矩形
typedef struct Rect
{
	int left;
	int top;
	int right;
	int bottom;
} Rect;

/// 点
typedef struct Point
{
	int x;
	int y;
} Point;

/// 尺寸
typedef struct Size
{
	int w;
	int h;
} Size;

/// 颜色
typedef struct Color
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} Color;

/// 直线
typedef struct Line
{
	Point start;
	Point end;
} Line;

/// 时间段结构
struct TimeSection
{
	int enable;			///< 使能
	int startHour;		///< 开始时间:小时
	int	startMinute;	///< 开始时间:分钟
	int	startSecond;	///< 开始时间:秒钟
	int	endHour;		///< 结束时间:小时
	int	endMinute;		///< 结束时间:分钟
	int	endSecond;		///< 结束时间:秒钟
};

/// IP 地址结构
union IpAddress
{
	uint8_t		c[4];
	uint16_t	s[2];
	uint32_t	l;
};

} // namespace Dahua


/// 保证32位和64位平台的指针变量长度一致，防止32位平台头文件增加指针导致64位下结构体大小改变
/// 用法：在指针变量后，使用此宏，N填任意值，同一个结构体内不重名即可。
/// char* pointer; UNUSED_FORCE_POINTER_ALIGN8(pointer);
#ifndef UNUSED_FORCE_POINTER_ALIGN8
#if defined(_WIN64) || (defined(__WORDSIZE) && __WORDSIZE==64)
#define UNUSED_FORCE_POINTER_ALIGN8(N)
#elif defined(_WIN32) || (defined(__WORDSIZE) && __WORDSIZE==32)
#define UNUSED_FORCE_POINTER_ALIGN8(N) uint32_t unused##N
#elif defined(__GNUC__)
#define UNUSED_FORCE_POINTER_ALIGN8(N) uint8_t unused##N[8-sizeof(void*)]
#else
#error "Can't find macro `__WORDSIZE' definition, please specify this macro 32 or 64 base on your platform!"
#endif
#endif

#endif// __DAHUA_TYPES_H__

