//
//  "$Id: ByteOrder.h 16503 2010-11-26 02:23:15Z wang_haifeng $"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __UTILS_NUMBER_STAT_H__
#define __UTILS_NUMBER_STAT_H__

#include <math.h>
#include "Defs.h"

namespace Dahua {
namespace Utils {


/// 浮点数统计类，可以统计最小值，最大值，平均值，标准差
class CNumberStat
{
public:
	CNumberStat()
	{
		clean();
	};
	virtual ~CNumberStat()
	{
	};

	void append(float number)
	{
		avg += number;
		dev += number * number ;
		max  = max > number  ?  max : number ;
		min  = min < number ?  min : number ;
		count++;
	}

	void clean()
	{
		max = 0 ;
		min = (float)(uint32_t)-1;
		avg = 0;
		count = 0;
		dev = 0;
	}

	float getMin()
	{
		return min;
	}

	float getMax()
	{
		return max;
	}

	float getAvg()
	{
		return avg /count ;
	}

	float getDev()
	{
		float tmp = avg /count;
		return 	(float)sqrt(dev / count - tmp * tmp );
	}
	
	int getCount() const
	{
		return count;
	}

private:
	float max ;
	float min ;
	float avg ;
	double dev ;
	int   count ;
};


} // namespace Utils
} // namespace Dahua

#endif// __UTILS_NUMBER_STAT_H__

