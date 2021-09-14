//
//  "$Id: SignalTemplate.h 52218 2012-03-01 03:36:35Z qin_fenglin $"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

/// \class SIGNAL_SIGNAL
/// \brief 信号类模块。
///
/// 一个信号被触发时，会调用所有连接到其上的TFunctionN对象，只支持
/// 返回值为void的TFunctionN。SIGNAL_SIGNAL是一个宏，根据参数个数会被替换成
/// TSignalN，用户通过 TSignalN<T1, T2, T3,..,TN>方式来使用，TN表示参数类型，
/// N表示函数参数个数，目前最大参数为6。
/// \see FUNCTION_FUNCTION

#define SIGNAL_SIGNAL DAHUA_JOIN(TSignal,SIGNAL_NUMBER)
#define FUNCTION_FUNCTION DAHUA_JOIN(TFunction, SIGNAL_NUMBER)

#if (SIGNAL_NUMBER != 0)
template <SIGNAL_CLASS_TYPES>
#endif
class SIGNAL_SIGNAL
{
	/// 信号节点状态
	enum SlotState
	{
		slotStateEmpty,		///< 节点为空
		slotStateNormal,	///< 节点以连接
	};

public:
	/// 节点挂载的位置
	enum SlotPosition
	{
		any,				///< 任意地方
		back,				///< 挂载在末尾
		front				///< 挂载在头部
	};

	/// 信号操作错误码
	enum ErrorCode
	{
		errorNoFound = -1,		///< 没有找到制定的对象
		errorExist = -2,		///< 对象已经存在
		errorFull = -3,			///< 已经到达能够连接的对象个数上限
		errorEmptyProc = -4,	///< 对象包含的函数指针为空，没有意义
	};

	/// 与信号模板参数类型匹配的函数指针对象类型
	typedef FUNCTION_FUNCTION<void SIGNAL_TYPES_COMMA> Proc;

private:
	/// 信号节点结构
	struct SignalSlot
	{
		Proc proc;
		SlotState state;
		bool running;
		uint32_t cost;
	};

	int m_numberMax;
	int m_number;
	SignalSlot* m_slots;
	CMutex m_mutex;
	int	m_threadId;

public:
	/// 构造函数
	/// \param maxSlots 能够连接的最大函数指针对象的个数
	SIGNAL_SIGNAL(int maxSlots) :
	  m_numberMax(maxSlots), m_number(0), m_threadId(-1)
	{
		m_slots = new SignalSlot[maxSlots];
		for(int i = 0; i < m_numberMax; i++)
		{
			m_slots[i].state = slotStateEmpty;
			m_slots[i].running = false;
		}
	}

	 /// 析构函数
	~SIGNAL_SIGNAL()
	{
		delete []m_slots;
	}

	/// 挂载函数指针对象
	/// \param proc 函数指针对象
	/// \param position 对象挂载位置
	/// \retval >=0 调用后已经挂载到信号的函数指针对象个数
	/// \retval <0 errorCode类型的错误码
	int attach(Proc proc, SlotPosition position = any)
	{
		int i;

		if(proc.empty())
		{
			return errorEmptyProc;
		}

		if(isAttached(proc))
		{
			return errorExist;
		}

		CGuard guard(m_mutex);

		switch(position)
		{
		case any:
			for(i = 0; i < m_numberMax; i++)
			{
				if(m_slots[i].state == slotStateEmpty)
				{
					m_slots[i].proc  = proc;
					m_slots[i].state = slotStateNormal;

					return ++m_number;
				}
			}
			break;
		case back:
			for(i = m_numberMax - 1; i >= 0; i--)
			{
				if(m_slots[i].state == slotStateEmpty)
				{
					for(int j = i; j < m_numberMax - 1; j++)
					{
						m_slots[j] = m_slots[j + 1];
					}
					m_slots[m_numberMax - 1].proc  = proc;
					m_slots[m_numberMax - 1].state = slotStateNormal;

					return ++m_number;
				}
			}
			break;
		case front:
			for(i = 0; i < m_numberMax; i++)
			{
				if(m_slots[i].state == slotStateEmpty)
				{
					for(int j = i; j > 0; j--)
					{
						m_slots[j] = m_slots[j - 1];
					}
					m_slots[0].proc  = proc;
					m_slots[0].state = slotStateNormal;

					return ++m_number;
				}
			}
			break;
		}

		return errorFull;
	}

	/// 卸载函数指针对象，根据对象中保存的函数指针来匹配。
	/// \param proc 函数指针对象
	/// \param wait 是否等待正在进行的回调结束。一般在使用者对象析构的时候需要等待，
	///             如果是在回调函数里卸载，则不能等待。等待要特别小心，防止死锁.
	/// \retval >=0 调用后已经挂载到信号的函数指针对象个数
	/// \retval <0 errorCode类型的错误码
	int detach(Proc proc, bool wait = false)
	{
		if (proc.empty())
		{
			return errorEmptyProc;
		}

		CGuard guard(m_mutex);

		for (int i = 0; i < m_numberMax; i++)
		{
			if(m_slots[i].proc ==  proc
				&& m_slots[i].state == slotStateNormal)
			{
				/// 回调线程和stop线程不是同一线程时，才需要等待，否则等待会引起自锁
				if(wait && m_slots[i].running && CThread::getCurrentThreadID() != m_threadId)
				{
					while(m_slots[i].running)
					{
						m_mutex.leave();
						CThread::sleep(1);
						infof("SIGNAL_SIGNAL::Detach wait callback exit!\n");
						m_mutex.enter();
					}
				}

				m_slots[i].state = slotStateEmpty;
				return --m_number;
			}
		};

		return errorNoFound;
	}

	/// 判断卸载函数指针对象是否挂载，根据对象中保存的函数指针来匹配。
	/// \param proc 函数指针对象
	bool isAttached(Proc proc)
	{
		CGuard guard(m_mutex);

		if(proc.empty())
		{
			return false;
		}

		for(int i = 0; i < m_numberMax; i++)
		{
			if(m_slots[i].proc ==  proc
				&& m_slots[i].state == slotStateNormal)
			{
				return true;
			}
		}

		return false;
	}

	/// 重载()运算符，可以以函数对象的形式来调用连接到信号的所有函数指针。
	inline void operator()(SIGNAL_TYPE_ARGS)
	{
		CGuard guard(m_mutex);
		uint64_t us1 = 0, us2 = 0;

		m_threadId = CThread::getCurrentThreadID(); // 保存回调线程ID

		for(int i = 0; i < m_numberMax; i++) // call back functions one by one
		{
			if(m_slots[i].state == slotStateNormal)
			{
				Proc temp = m_slots[i].proc;

				m_slots[i].running = true;
				m_mutex.leave();

				// 函数执行与性能统计
	 			us1 = CTime::getCurrentMicroSecond();
				temp(SIGNAL_ARGS);
				us2 = CTime::getCurrentMicroSecond();
				m_slots[i].cost = (us1 <= us2) ? uint32_t(us2 - us1) : 1;

				m_mutex.enter();
				m_slots[i].running = false;
			}
		}
	}

	void stat()
	{
		int i;

		CGuard guard(m_mutex);

		for(i = 0; i < m_numberMax; i++)
		{
			if(m_slots[i].state == slotStateNormal)
			{
				printf("\t%8d us, %p, %s\n",
				 m_slots[i].cost,
				 m_slots[i].proc.getObject(),
				 m_slots[i].proc.getObjectType());
			}
		}
	}
};

#undef SIGNAL_SIGNAL
#undef FUNCTION_FUNCTION
