//
//  "$Id: Time.h 109610 2013-04-02 06:41:41Z qin_fenglin $"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INFRA3_TIME_WRAPPER_H__
#define __INFRA3_TIME_WRAPPER_H__

#include <string>
#include "Defs.h"
#include "IntTypes.h"
#include "String.h"
#include "Function.h"

namespace Dahua {
namespace Infra {

/// 本地时间，包含时区、夏令时等因素
struct SystemTime
{
	int  year;		///< 年。
	int  month;		///< 月，January = 1, February = 2, and so on.
	int  day;		///< 日。
	int  wday;		///< 星期，Sunday = 0, Monday = 1, and so on
	int  hour;		///< 时。
	int  minute;	///< 分。
	int  second;	///< 秒。
	int  isdst;		///< 夏令时标识。<0表示未知 0不在夏令时 >0处在夏令时
};

/// 本地时间，包含毫秒的SystemTime增强版
struct SystemTimeEx
{
	int  year;		///< 年。
	int  month;		///< 月，January = 1, February = 2, and so on.
	int  day;		///< 日。
	int  wday;		///< 星期，Sunday = 0, Monday = 1, and so on
	int  hour;		///< 时。
	int  minute;	///< 分。
	int  second;	///< 秒。
	int  millisecond; ///<毫秒
	int  isdst;		///< 夏令时标识。<0表示未知 0不在夏令时 >0处在夏令时
	int  reserved1;
	int  reserved2;
	int  reserved3;
};

/// \class CTime
/// \brief 时间类，实现了本地时间存取，运算，格式控制与打印等接口
/// CTime类处理的时间都是本地时间，在GMT时间的基础上加了时区偏移。
class INFRA_API CTime : public SystemTime
{
public:
	/// 时间修改回调函数
	typedef TFunction1<bool, SystemTime> ModifyProc;

	/// 时间修改函数
	typedef TFunction2<void, const CTime&, int> SetCurrentTimeHook;
	typedef TFunction2<void, const SystemTimeEx&, int> SetCurrentTimeHookEx;

	/// 日期顺序格式
	enum DateFormat
	{
		ymd,
		mdy,
		dmy
	};

	/// 格式化模式选项
	enum FormatMask
	{
		fmGeneral = 0,		///< 只指定是否显示以及起始值
		fmSeparator = 1,	///< 指定分隔符
		fmDateFormat = 2,	///< 指定年月日顺序
		fmHourFormat = 4,	///< 指定小时制式

		fmAll = fmSeparator | fmDateFormat | fmHourFormat	///< 指定所有格式选项
	};

	/// 缺省构造函数
	CTime( );

	/// 内部会尝试转换到本地时区，但由于时区文件被裁剪，在不同平台结果不一样
	/// \param time UNIX Epoch秒数
	CTime(uint64_t time);

	///把时间转成到对象内部，考不考虑时区文件可选
	/// \param time UNIX Epoch秒数
	/// \param addTimezone true 表示关心时区文件,自动在time基础上加上时区时差；false表示不关心时区文件，纯粹做时间转化
	CTime(uint64_t time,bool addTimezone);

	/// 普通时间构造函数
	/// 不涉及时区计算，在任何平台上的行为一致
	CTime( int vyear, int vmonth, int vday, int vhour, int vmin, int vsec);

	/// 尝试得到UNIX Epoch时间，指从GMT 1970-1-1 00:00:00 到某个时刻经过的秒数
	/// 注意：本函数在计算时会尝试修正到0时区，由于嵌入式系统被裁剪时区文件，无法达到修正效果
	/// \return UNIX Epoch时间
	uint64_t makeTime() const;

	/// 将UNIX Epoch秒数分解成年月日时分秒
	/// 注意：本函数在转换时会先尝试添加时区偏移量，再进行年月日的转换。由于部分系统时区文件被裁剪，导致结果可能不是预期
	/// \param time UNIX Epoch秒数
	void breakTime(uint64_t time);

	/// 时间调节
	/// \param seconds 调节的秒数
	/// \return 调节后新的时间对象
	CTime operator +( int64_t seconds ) const;

	/// 时间调节
	/// \param seconds 调节的秒数
	/// \return 调节后新的时间对象
	CTime operator -( int64_t seconds ) const;

	/// 时间差运算
	/// \param time 相比较的时间
	/// \return 比较的结果，为(*this - time)得到的秒数
	int64_t operator -( const CTime& time ) const;

	/// 时间调节
	/// \param seconds 调节的秒数
	/// \return 对象本身
	CTime& operator +=( int64_t seconds );

	/// 时间调节
	/// \param seconds 调节的秒数
	/// \return 对象本身
	CTime& operator -=( int64_t seconds );

	/// 时间比较
	/// \param time 相比较的时间
	/// \return 相等返回true，否则返回false
	bool operator == (const CTime& time) const;

	/// 时间比较
	/// \param time 相比较的时间
	/// \return 不等返回true，否则返回false
	bool operator != (const CTime& time) const;

	/// 时间比较
	/// \param time 相比较的时间
	/// \return <返回true，否则返回false
	bool operator < (const CTime& time) const;

	/// 时间比较
	/// \param time 相比较的时间
	/// \return <=返回true，否则返回false
	bool operator <= (const CTime& time) const;

	/// 时间比较
	/// \param time 相比较的时间
	/// \return >返回true，否则返回false
	bool operator > (const CTime& time) const;

	/// 时间比较
	/// \param time 相比较的时间
	/// \return >=返回true，否则返回false
	bool operator >= (const CTime& time) const;

	/// 时间格式化
	/// \param buf 字符串缓冲，一定要足够大
	/// \param format 格式化字符串，如"yyyy-MM-dd HH:mm:ss tt"
	/// yy = 年，不带世纪 yyyy = 年，带世纪
	/// M = 非0起始月 MM = 0起始月 MMMM = 月名称
	/// d = 非0起始日 dd = 0起始日
	/// H = 非0起始24小时 HH = 0起始24小时 h = 非0起始12小时 hh = 0起始12小时
	/// tt = 显示上午或下午
	/// \param mask 格式选项，指定日期分隔符，年月日顺序，小时制式是否由统一的格
	/// 式决定。相应位置0，表示使用统一格式，置1，表示使用format指定的格式
	void format(char* buf, const char *format = "yyyy-MM-dd HH:mm:ss", int mask = fmGeneral) const;

	/// 时间字符串解析
	/// \param buf 输入的字符串缓冲
	/// \param format 格式化字符串，暂时只支持默认的"yyyy-MM-dd HH:mm:ss"
	/// \param mask 格式选项，指定日期分隔符，年月日顺序，小时制式是否由统一的格
	/// 式决定。相应位置0，表示使用统一格式，置1，表示使用format指定的格式
	/// \return 是否成功
	bool parse(const char* buf, const char *format = "yyyy-MM-dd HH:mm:ss", int mask = fmGeneral);

	/// 会立刻实际的去取本地当前系统时间,更精确和及时
	/// 但linux下多线程调用频繁可能会导致较高cpu，适用于时间秒跳变准确，且调用不频繁的场景
	static CTime getCurrentTime();

	/// 会立刻实际的去取本地当前系统时间,更精确和及时，精确到毫秒
	/// 但linux下多线程调用频繁可能会导致较高cpu，适用于时间秒跳变准确，且调用不频繁的场景
	static SystemTimeEx getCurrentTimeEx();

	/// 返回保存的当前的系统时间，内部会间隔去刷新保存的本地当前系统时间
	/// 用于多线程频繁打印，与实际时间精度控制在1s偏差范围内，但是cpu占用率低
	static CTime getCurTimeForPrint();

	/// 设置本地当前系统时间
	/// \param time 新的时间
	/// \param toleranceSeconds 容差，表示容许设置时间和当前差多少秒内不做修改
	static void setCurrentTime(const CTime& time, int toleranceSeconds = 0);

	/// 设置本地当前系统时间并返回设置成功与否
	/// \param time 新的时间
	/// \param toleranceSeconds 容差，表示容许设置时间和当前差多少秒内不做修改
	static bool setCurrentTimeEx(const CTime& time, int toleranceSeconds = 0);

	/// 设置本地当前系统时间
	/// \param time 新的支持毫秒时间
	/// \param toleranceSeconds 容差，表示容许设置时间和当前差多少秒内不做修改
	static bool setCurrentTime(const SystemTimeEx& time, int toleranceSeconds = 0);

	/// 得到从系统启动到现在的毫秒数
	static uint64_t getCurrentMilliSecond();

	/// 得到从系统启动到现在的微秒数
	static uint64_t getCurrentMicroSecond();

	/// 设置时间格式，会影响Format的输出的字符串格式，如"yyyy-MM-dd HH:mm:ss"
	static void setFormatString(const char* format);

	/// 获取时间格式
	static const CString getFormatString();

	/// 获取日期格式
	static DateFormat getDateFormat();

	/// 获取当前是否是12小时制
	static bool get12Hour();

	/// 获取日期分割符
	static char getSeparator();

	/// 注册修改时间回调函数，这个在上层调用了setCurrentTime后会被同步触发
	/// \param proc 时间修改回调函数
	/// \return 注册时间修改回调函数是否成功
	static bool attachModifyProc(ModifyProc proc);

	/// 注册用户自定义修改时间函数钩子，调用setCurrentTime时实际调用proc
	/// \param proc 时间修改钩子函数
	/// \return setCurrentTime原来的实现函数指针
	static SetCurrentTimeHook attachSetCurrentTimeHook(SetCurrentTimeHook proc);

	/// 注册用户自定义修改时间函数钩子，调用setCurrentTimeEx时实际调用proc
	/// \param proc 时间修改钩子函数
	/// \return setCurrentTimeEx原来的实现函数指针
	static SetCurrentTimeHookEx attachSetCurrentTimeHookEx(SetCurrentTimeHookEx proc);

	/// 设置时间格式，会影响Format的输出的字符串格式，如"yyyy-MM-dd HH:mm:ss"
	static void setFormat(const CString2& format);

	/// 获取时间格式
	static const CString2 getFormat();

	/// ！！注意：虽然接口名UTCtime，但返回的不一定是UTC时间，取决于系统环境(时区文件是否被裁剪)！！
	/// 获取未经本地化处理的时间，执行速度比getCurrentTime快
	static uint64_t getCurrentUTCtime();

	/// ！！注意：虽然接口名UTCtime，但返回的不一定是UTC时间，取决于系统环境(时区文件是否被裁剪)！！
	/// 获取精确到毫秒的相对时间
	/// 返回值: 时间毫秒数
	static uint64_t getCurrentUTCMilliSecond();

	/// 禁止时区，优化性能。CTime中不进行时区转换，也不获取时区
	static int disableTimezone();

	/// 设置夏令时标志
	/// \param[in] flag true 处在夏令时 false 不在夏令时
	static bool setDst(bool flag);

	/// ！！只有裁剪时区文件的系统可以调用, Windows或服务器环境不要使用！！
	/// 设置本地时区(不受夏令时影响)
	/// \param[in] diffSecond 本地时区和0时区的差值，东半球正值，西半球负值，单位秒。
	/// 如纽约的时区是西5区，不论是否在夏令时，传值-5*3600
	static bool setTimeZone(int diffSecond);

	/// UTC时间定义 32字节
	struct UTCTimeDef
	{
		uint64_t second;	/// UTC时间 以1970年1月1日凌晨格林威冶时间以后的秒数
		int diffSecond;		/// 本地时区值，单位秒。详见 setTimeZone 接口说明
		int isdst;			/// 夏令时标识。0不在夏令时 >0处在夏令时
		int reserved[4];
	};

	/// ！！只有裁剪时区文件的系统可以调用, Windows或服务器环境不要使用！！
	/// 适用于嵌入式设备, 返回UTC时间和本地时间的差值，必须先调用 setDst和setTimeZone才能调用成功
	/// 由于UTC时间精度要求不高，采用类似getCurTimeForPrint方式。
	/// \param[out] utc 时间定义
	/// return true 获取UTC时间成功 false 获取失败，原因是时区或夏令时未设置
	static bool  getUTCForEmbed(struct UTCTimeDef &utc);

	/// 获取夏令时标志
	//返回值：-1表示不确定，0-表示不是夏令时，1-表示是夏令时
	static int  getDst();
	
	/// 获取本地时区时差(读取系统时区文件)
	/// 返回值:本地时间与 GMT 时间相差的秒数; 本地时间 + 相差的秒数 = GMT时间
	static long getTimeZone ();

	/// 转换秒数接口
	/// \param[in] time结构体
	/// 返回值：time结构体转化的秒数，不考虑时区，只是纯粹的时间转换。
	static uint64_t timeToSeconds(SystemTime const& time);
	
	/// 时区时差查询
	/// \param[in] zoneIndex 时区索引与设备配置对应
	/// 返回值：与0时区的时差，单位为s。
	static int getTimeZonefromTable(int zoneIndex);



public:
	static const CTime minTime; ///< 有效的最小时间
	static const CTime maxTime; ///< 有效的最大时间

};

} // namespace Infra
} // namespace Dahua

#endif // __INFRA_TIME_WRAPPER_H__
