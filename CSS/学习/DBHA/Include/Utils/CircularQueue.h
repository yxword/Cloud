//
//  "$Id: CircularQueue.h 16503 2010-11-26 02:23:15Z wang_haifeng $"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __DAHUA_CIRCULAR_QUEUE_H__
#define __DAHUA_CIRCULAR_QUEUE_H__

#include <assert.h>
#include <memory>

namespace Dahua {
namespace Utils {


/// 环形队列模板类
template <class T, class A = std::allocator<T> > class TCircularQueue
{
	TCircularQueue(TCircularQueue const& other);
	TCircularQueue& operator=(TCircularQueue const& other);

public:
	TCircularQueue(int maxSize)
	{
		assert(maxSize >= 1);

		m_queue = m_allocator.allocate(maxSize);

		assert(m_queue);

		m_front = 0;
		m_back = 0;
		m_begin = 0;
		m_end = 0;
		m_maxSize = maxSize;
		m_size = 0;
	};

	~TCircularQueue()
	{
		clear();
		m_allocator.deallocate(m_queue, m_maxSize);
	}

	/// 判断队列是否为空
	bool empty()
	{
		return (m_size == 0);
	}

	/// 判断队列是否为满
	bool full()
	{
		return (m_size == m_maxSize);
	}

	/// 获取队列元素个数
	int size()
	{
		return m_size;
	}

	/// 获取队列能够存放的最大元素个数
	int getMaxSize()
	{
		return m_maxSize;
	}

	/// 获取队首元素
	const T & front()
	{
		assert(m_size > 0);

		return m_queue[m_front];
	}

	/// 获取队尾元素
	const T & back()
	{
		assert(m_size > 0);

		return m_queue[(m_back + m_maxSize - 1) % m_maxSize];
	}

	/// 元素放到队尾
	void push(const T &item)
	{
		assert(m_size < m_maxSize);

#ifdef __MSVC__
		m_allocator.construct((A::rebind<T>::other::pointer)&m_queue[m_back], item);
#else
		m_allocator.construct(&m_queue[m_back], item);
#endif
		m_size++;
		m_back = (m_back + 1) % m_maxSize; //m_back始终指向最后一个元素的下一个位置
		m_end++;
	}

	/// 队首元素出列
	void pop()
	{
		assert(m_size > 0);

#ifdef __MSVC__
		m_allocator.destroy((A::rebind<T>::other::pointer)&m_queue[m_front]);
#else
		m_allocator.destroy(&m_queue[m_front]);
#endif
		m_size--;
		m_front = (m_front + 1) % m_maxSize; //m_front移向下一位置
		m_begin++;
	}

	/// 使用元素下标访问， 下标必须介于0和m_size之间
	const T & operator [] (int pos)
	{
		assert (pos >= 0 && pos < m_size);

		return m_queue[(pos + m_front) % m_maxSize];
	}

	/// 清空队列
	void clear()
	{
		while(m_size)
		{
			pop();
		}
		m_back = m_front = 0;
		m_begin = m_end; // 不能清零，序号要始终保持递增
		m_size = 0;
	}

	/// 获取队首元素序列号
	int begin()
	{
		return m_begin;
	}

	/// 获取队尾元素序列号
	int end()
	{
		return m_end;
	}

	/// 使用序列号访问数据
	/// \param sn 序列号，必须介于m_begin和m_end之间
	const T & look(int sn)
	{
		return operator[](sn - m_begin);
	}

private:
	T * m_queue;	///< 存放队列元素的指针(数组)
	int m_maxSize;	///< 队列容量
	int m_front;	///< 队首位置
	int m_back;		///< 队尾位置(最后一个元素的下一位置)
	int m_size;		///< 添加数据的个数
	int	m_begin;	///< 队首元素序列号，随着push调用不断增加，可能会超过m_maxSize
	int m_end;		///< 队尾元素序列号，随着pop调用不断增加，可能会超过m_maxSize
	A m_allocator;
};

} // namespace Utils
} // namespace Dahua

#endif // __DAHUA_CIRCULAR_QUEUE_H__

