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


/// ���ζ���ģ����
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

	/// �ж϶����Ƿ�Ϊ��
	bool empty()
	{
		return (m_size == 0);
	}

	/// �ж϶����Ƿ�Ϊ��
	bool full()
	{
		return (m_size == m_maxSize);
	}

	/// ��ȡ����Ԫ�ظ���
	int size()
	{
		return m_size;
	}

	/// ��ȡ�����ܹ���ŵ����Ԫ�ظ���
	int getMaxSize()
	{
		return m_maxSize;
	}

	/// ��ȡ����Ԫ��
	const T & front()
	{
		assert(m_size > 0);

		return m_queue[m_front];
	}

	/// ��ȡ��βԪ��
	const T & back()
	{
		assert(m_size > 0);

		return m_queue[(m_back + m_maxSize - 1) % m_maxSize];
	}

	/// Ԫ�طŵ���β
	void push(const T &item)
	{
		assert(m_size < m_maxSize);

#ifdef __MSVC__
		m_allocator.construct((A::rebind<T>::other::pointer)&m_queue[m_back], item);
#else
		m_allocator.construct(&m_queue[m_back], item);
#endif
		m_size++;
		m_back = (m_back + 1) % m_maxSize; //m_backʼ��ָ�����һ��Ԫ�ص���һ��λ��
		m_end++;
	}

	/// ����Ԫ�س���
	void pop()
	{
		assert(m_size > 0);

#ifdef __MSVC__
		m_allocator.destroy((A::rebind<T>::other::pointer)&m_queue[m_front]);
#else
		m_allocator.destroy(&m_queue[m_front]);
#endif
		m_size--;
		m_front = (m_front + 1) % m_maxSize; //m_front������һλ��
		m_begin++;
	}

	/// ʹ��Ԫ���±���ʣ� �±�������0��m_size֮��
	const T & operator [] (int pos)
	{
		assert (pos >= 0 && pos < m_size);

		return m_queue[(pos + m_front) % m_maxSize];
	}

	/// ��ն���
	void clear()
	{
		while(m_size)
		{
			pop();
		}
		m_back = m_front = 0;
		m_begin = m_end; // �������㣬���Ҫʼ�ձ��ֵ���
		m_size = 0;
	}

	/// ��ȡ����Ԫ�����к�
	int begin()
	{
		return m_begin;
	}

	/// ��ȡ��βԪ�����к�
	int end()
	{
		return m_end;
	}

	/// ʹ�����кŷ�������
	/// \param sn ���кţ��������m_begin��m_end֮��
	const T & look(int sn)
	{
		return operator[](sn - m_begin);
	}

private:
	T * m_queue;	///< ��Ŷ���Ԫ�ص�ָ��(����)
	int m_maxSize;	///< ��������
	int m_front;	///< ����λ��
	int m_back;		///< ��βλ��(���һ��Ԫ�ص���һλ��)
	int m_size;		///< ������ݵĸ���
	int	m_begin;	///< ����Ԫ�����кţ�����push���ò������ӣ����ܻᳬ��m_maxSize
	int m_end;		///< ��βԪ�����кţ�����pop���ò������ӣ����ܻᳬ��m_maxSize
	A m_allocator;
};

} // namespace Utils
} // namespace Dahua

#endif // __DAHUA_CIRCULAR_QUEUE_H__

