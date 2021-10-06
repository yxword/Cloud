//
//  "$Id$"
//
//  Copyright (c)1992-2011, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

///	\file Utils/IntervalSet.h
///	Interval 集合容器，用于文件列表播放器和时间轴之间传递数据，行为与 boost.interval_set 类似。
///

#ifndef UTILS_INTERVAL_SET_H__
#define UTILS_INTERVAL_SET_H__


#include <utility>	// for std::pair


namespace Dahua {
namespace Utils {

/// 时间区间集合(主要用于回放时间控制)
/// \n 每个时间区间有两个关键时间点:开始时间和结束时间
class CIntervalSet
{
public:
	/// 时间区间(时间段)类型; 表示[first, second)一段时间; 单位为秒
	typedef std::pair<int, int> Interval;

	/// 默认构造函数
	CIntervalSet();

	/// 拷贝构造函数
	CIntervalSet(CIntervalSet const& rhs);

	/// 析构函数
	~CIntervalSet();

	/// 插入时间区间; 这个操作可能会造成原有区间的合并,使区间数减少
	void insert(Interval const& range);

	/// 删除时间区间; 这个操作可能会造成原有区间的分裂,使区间数增加
	void erase(Interval const& range);

	/// 清空时间表
	void clear();

	/// 判断集合是否为空
	bool empty() const;

	/// 取得时间区间数
	int count() const;

	/// 取得第index个时间区间
	Interval get(int index) const;

	/// \brief 使用二分查找法查找时间区间,返回包含指定时间的时间区间索引.
	///	\n 如果时间表中没有区间包含这个时间,返回比这个时间晚的最近的一个时间区间
	///	\n 的索引,如果时间表中没有比这个时间晩的区间,返回-1.
	///
	/// \param time 要查找的时间区间包含这个时间
	/// \return 返回找到的时间区间的索引.
	///
	int find(int time) const;

	/// 交换时间表
	void swap(CIntervalSet& rhs);

	/// 重载赋值运算符
	CIntervalSet& operator=(CIntervalSet const& rhs);

	/// 合并时间表
	CIntervalSet& operator+=(CIntervalSet const&);

private:
	struct Internal;
	Internal*	m_internal;
};

} // namespace Utils
} // namespace Dahua


#endif // UTILS_INTERVAL_SET_H__
