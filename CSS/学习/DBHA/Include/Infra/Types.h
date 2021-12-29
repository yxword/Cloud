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

/// ����
typedef struct Rect
{
	int left;
	int top;
	int right;
	int bottom;
} Rect;

/// ��
typedef struct Point
{
	int x;
	int y;
} Point;

/// �ߴ�
typedef struct Size
{
	int w;
	int h;
} Size;

/// ��ɫ
typedef struct Color
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} Color;

/// ֱ��
typedef struct Line
{
	Point start;
	Point end;
} Line;

/// ʱ��νṹ
struct TimeSection
{
	int enable;			///< ʹ��
	int startHour;		///< ��ʼʱ��:Сʱ
	int	startMinute;	///< ��ʼʱ��:����
	int	startSecond;	///< ��ʼʱ��:����
	int	endHour;		///< ����ʱ��:Сʱ
	int	endMinute;		///< ����ʱ��:����
	int	endSecond;		///< ����ʱ��:����
};

/// IP ��ַ�ṹ
union IpAddress
{
	uint8_t		c[4];
	uint16_t	s[2];
	uint32_t	l;
};

} // namespace Dahua


/// ��֤32λ��64λƽ̨��ָ���������һ�£���ֹ32λƽ̨ͷ�ļ�����ָ�뵼��64λ�½ṹ���С�ı�
/// �÷�����ָ�������ʹ�ô˺꣬N������ֵ��ͬһ���ṹ���ڲ��������ɡ�
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

