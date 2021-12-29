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
/// \brief �̰߳�ȫ�����ȼ���Ϣ����ģ���࣬��ʵ�����ɽ��ܸ�������������Ϣ����Ϣ�����ࡣ
///
/// ʾ����������:
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

	/// ���캯��
	/// \param queueSize ��Ϣ���еĳ���
	/// \param prioLevels ���ȼ��ȼ������ȼ���Խ�٣���Ϣ�����ٶ�Խ��
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

	/// ��������
	virtual ~TMsgQue()
	{
		delete []m_prioIterator;
	};

	/// ������Ϣ
	/// \param message ��Ϣ����ָ��
	/// \param priority ���͵����ȼ���ֻ����0��prioLevels-1֮��
	/// \return ���ͽ���������Ϣ�����������������ȼ����ԣ�����ʧ��
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

	/// ������Ϣ
	/// \param message ��Ϣ����ָ��
	/// \param wait ����Ϣ����Ϊ�յ�������Ƿ�����
	/// \param deque ������Ϣ���Ƿ�����Ϣ����
	/// \return ���ս���������Ϣ����Ϊ�գ����Ƿ�����ģʽ���򷵻�ʧ��
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

	/// �õ���Ϣ��������Ϣ����
	int size()
	{
		Infra::CGuard guard(m_mutex);

		return m_queue.size();
	}

	/// �õ���Ϣ���е���󳤶�
	int getMaxSize()
	{
		Infra::CGuard guard(m_mutex);

		return m_queueSize;
	}

	/// �õ���Ϣ���е���󳤶ȣ������´�SendMessageʱ��Ч
	/// \param size �µ���Ϣ���г��ȣ����Ա�ԭ���Ĵ����С
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
