//
//  "$Id: MsgQue.h 16503 2010-11-26 02:23:15Z wang_haifeng $"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef DAHUA_MSG_QUE_H__
#define DAHUA_MSG_QUE_H__

#include "Infra/Guard.h"
#include "Infra/Semaphore.h"
#include "Memory/PoolAllocator.h"

#if defined(_MSC_VER)
	#pragma warning (push)
	#pragma warning (disable : 4786)
#endif
#include <list>

namespace Dahua {
namespace Utils {

/// \class TMsgQue
/// \brief 线程安全的优先级消息队列模板类，能实例化成接受各种数据类型消息的消息队列类。
///
/// 示例代码如下:
/// \code
/// struct NetMessage;
/// struct GUIMessage;
///
/// TMsgQue<NetMessage> myNetMsgQue;
/// TMsgQue<GUIMessage> myGUIMsgQue;
///
/// NetMessage msg;
/// makeNetMessage(&msg);
/// myNetMsgQue.sendMessage(msg);
/// \endcode
template<typename M>
class TMsgQue
{
public:

	/// 构造函数
	/// \param queueSize 消息队列的长度
	/// \param prioLevels 优先级等级数，等级数越少，消息处理速度越快
	TMsgQue(int queueSize = 1024, int prioLevels = 1)
	{
		m_queueSize = queueSize;
		m_prioLevels = prioLevels;
		m_prioIterator = new Iterator[prioLevels];
		for (int i = 0; i < prioLevels; i++)
		{
			m_prioIterator[i] = m_queue.begin();
		}
	};

	/// 析构函数
	virtual ~TMsgQue()
	{
		delete []m_prioIterator;
	};

	/// 发送消息
	/// \param message 消息对象指针
	/// \param priority 发送的优先级，只能在0到prioLevels-1之间
	/// \return 发送结果，如果消息队列已满，或者优先级不对，调用失败
	bool sendMessage (M& message, int priority = 0)
	{
		Iterator oldIterator, newIterator;

		if(priority < 0 || priority >= m_prioLevels)
		{
			return false;
		}

		m_mutex.enter();
		if(m_queue.size() >= (size_t)m_queueSize)
		{
			m_mutex.leave();
			return false;
		}

		oldIterator = m_prioIterator[priority];
		newIterator = m_queue.insert(oldIterator, message);

		for(int i = priority + 1; i < m_prioLevels; i++)
		{
			if(m_prioIterator[i] == oldIterator)
			{
				m_prioIterator[i] = newIterator;
			}
			else
			{
				break;
			}
		}
		m_semaphore.post();
		m_mutex.leave();
		return true;
	};

	/// 接收消息
	/// \param message 消息对象指针
	/// \param wait 在消息队列为空的情况下是否阻塞
	/// \param deque 接受消息后是否让消息出列
	/// \return 接收结果，如果消息队列为空，又是非阻塞模式，则返回失败
	bool recvMessage (M& message, bool wait = true, bool deque = true)
	{
		Infra::CGuard guard(m_mutex);

		if(m_semaphore.tryPend() != 0){
			if(wait){
				m_mutex.leave();
				m_semaphore.pend();
				m_mutex.enter();
			}else{
				return false;
			}
		}

		message = m_queue.front();

		if(deque)
		{
			for(int i = m_prioLevels - 1; i >= 0; i--)
			{
				if(m_prioIterator[i] == m_queue.begin())
				{
					m_prioIterator[i]++;
				}
				else
				{
					break;
				}
			}
			m_queue.pop_front();
		}
		else
		{
			m_semaphore.post();
		}

		return true;
	}

	void clean()
	{
		Infra::CGuard guard(m_mutex);
		while(!m_queue.empty())
		{
			if(m_semaphore.tryPend() == 0){
				m_queue.pop_back();
			}else{
				break;
			}
		}
	}

	/// 得到消息队列中消息个数
	int size()
	{
		Infra::CGuard guard(m_mutex);

		return m_queue.size();
	}

	/// 得到消息队列的最大长度
	int getMaxSize()
	{
		Infra::CGuard guard(m_mutex);

		return m_queueSize;
	}

	/// 得到消息队列的最大长度，将在下次SendMessage时生效
	/// \param size 新的消息队列长度，可以比原来的大或者小
	void setMaxSize(int size)
	{
		Infra::CGuard guard(m_mutex);

		m_queueSize = size;
	}
private:
	typedef std::list<M, Memory::TPoolAllocator<M> > Queue;
	typedef typename Queue::iterator Iterator;

	Queue m_queue;
	Iterator* m_prioIterator;
	Infra::CMutex m_mutex;
	Infra::CSemaphore m_semaphore;
	int m_queueSize;
	int m_prioLevels;
};

} // namespace Utils
} // namespace Dahua

#if defined(_MSC_VER)
	#pragma warning (pop)
#endif

#endif// DAHUA_MSG_QUE_H__
