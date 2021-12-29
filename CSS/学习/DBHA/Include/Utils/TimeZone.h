//
//  "$Id: TimeZone.h 355685 2016-06-24 02:34:51Z 20429 $"
//
//  Copyright (c)1992-2012, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description: Time Zone definition
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __DAHUA_UTILS_TIMEZONE_H__
#define __DAHUA_UTILS_TIMEZONE_H__

namespace Dahua {
namespace Utils {

struct TimeZone
{
	const char* str;	///< 时区描述
	int zone;			///< 与0时区相差的秒数
};

/// 注意：此处定义变量为静态变量，推荐先封装成函数再调用，减少各处引起导致的体积变大
static const TimeZone sg_cfgTimeZoneStringList[] = {
	{"GMT+00:00",					0/*0*3600*/},
	{"GMT+01:00",					1*3600},
	{"GMT+02:00",					2*3600},
	{"GMT+03:00",					3*3600},
	{"GMT+03:30",					3.5*3600},
	{"GMT+04:00",					4*3600},
	{"GMT+04:30",					4.5*3600},
	{"GMT+05:00",					5*3600},
	{"GMT+05:30",					5.5*3600},
	{"GMT+05:45",					5.75*3600},
	{"GMT+06:00",					6*3600},
	{"GMT+06:30",					6.5*3600},
	{"GMT+07:00",					7*3600},
	{"GMT+08:00",					8*3600},
	{"GMT+09:00",					9*3600},
	{"GMT+09:30",					9.5*3600},
	{"GMT+10:00",					10*3600},
	{"GMT+11:00",					11*3600},
	{"GMT+12:00",					12*3600},
	{"GMT+13:00",					13*3600},
	{"GMT-01:00",					-1*3600},
	{"GMT-02:00",					-2*3600},
	{"GMT-03:00",					-3*3600},
	{"GMT-03:30",					-3.5*3600},
	{"GMT-04:00",					-4*3600},
	{"GMT-05:00",					-5*3600},
	{"GMT-06:00",					-6*3600},
	{"GMT-07:00",					-7*3600},
	{"GMT-08:00",					-8*3600},
	{"GMT-09:00",					-9*3600},
	{"GMT-10:00",					-10*3600},
	{"GMT-11:00",					-11*3600},
	{"GMT-12:00",					-12*3600},
	{"GMT-4:30",					-4.5*3600},
	{"GMT+10:30",					10.5*3600},
	{"GMT+14:00",					14*3600},
	{NULL,							0}
};

}
}

#endif /// __DAHUA_UTILS_TIMEZONE_H__
