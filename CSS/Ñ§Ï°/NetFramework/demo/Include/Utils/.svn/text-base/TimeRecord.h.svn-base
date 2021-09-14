//
//  "$Id$"
//
//  Copyright (c)1992-2011, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef UTILS_TIME_RECORD_H__
#define UTILS_TIME_RECORD_H__

#include <stdio.h>
#include "Infra/IntTypes.h"
#include "Infra/Time.h"


namespace Dahua {
namespace Utils {


/// \class CTimeRecord
/// \brief 时间记录类，用于测试统计多个采样点之间的时间差
class CTimeRecord
{
	/// 时间采样点结构
	typedef struct
	{
		const char *name;
		uint64_t us;
	} TimePoint;

public:

	/// 构造函数
	/// size预计采样点个数，实际采样次数应小于预设值
	CTimeRecord(const char* name, int size) : m_name(name), m_size(size), m_pos(0)
	{
		m_points = new TimePoint[size];
	};

	/// 析构函数
	~CTimeRecord()
	{
		delete []m_points;
	};

	/// 清空采样记录
	void reset()
	{
		m_pos = 0;
	};

	/// 执行采样
	/// \param name 采样点名称标记
	void sample(const char* name)
	{
		m_points[m_pos].name = name;
		m_points[m_pos].us = Infra::CTime::getCurrentMicroSecond();
		m_pos++;
	}

	/// 执行最后一次采样，并统计采样结果
	/// \param timeout 超时后打印，单位微妙，0表示总是打印
	void stat(uint32_t timeout = 0);

private:
	TimePoint* m_points;
	const char* m_name;
	int m_size;
	int m_pos;
};


inline void CTimeRecord::stat(uint32_t timeout)
{
	sample(NULL);
	if(!timeout || m_points[m_pos - 1].us - m_points[0].us >= timeout)
	{
		for(int i = 1; i < m_pos; i++)
		{
			printf("%s-Per-%s-%s : %d us\n", m_name, m_points[i - 1].name, m_points[i].name, (i == 0) ? 0 : (int)(m_points[i].us - m_points[i-1].us));
		}
	}
};


} // namespace Utils
} // namespace Dahua

#endif // UTILS_TIME_RECORD_H__


