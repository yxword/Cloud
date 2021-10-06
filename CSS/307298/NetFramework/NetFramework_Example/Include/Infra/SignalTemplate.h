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
/// \brief �ź���ģ�顣
///
/// һ���źű�����ʱ��������������ӵ����ϵ�TFunctionN����ֻ֧��
/// ����ֵΪvoid��TFunctionN��SIGNAL_SIGNAL��һ���꣬���ݲ��������ᱻ�滻��
/// TSignalN���û�ͨ�� TSignalN<T1, T2, T3,..,TN>��ʽ��ʹ�ã�TN��ʾ�������ͣ�
/// N��ʾ��������������Ŀǰ������Ϊ6��
/// \see FUNCTION_FUNCTION

#define SIGNAL_SIGNAL DAHUA_JOIN(TSignal,SIGNAL_NUMBER)
#define FUNCTION_FUNCTION DAHUA_JOIN(TFunction, SIGNAL_NUMBER)

#if (SIGNAL_NUMBER != 0)
template <SIGNAL_CLASS_TYPES>
#endif
class SIGNAL_SIGNAL
{
	/// �źŽڵ�״̬
	enum SlotState
	{
		slotStateEmpty,		///< �ڵ�Ϊ��
		slotStateNormal,	///< �ڵ�������
	};

public:
	/// �ڵ���ص�λ��
	enum SlotPosition
	{
		any,				///< ����ط�
		back,				///< ������ĩβ
		front				///< ������ͷ��
	};

	/// �źŲ���������
	enum ErrorCode
	{
		errorNoFound = -1,		///< û���ҵ��ƶ��Ķ���
		errorExist = -2,		///< �����Ѿ�����
		errorFull = -3,			///< �Ѿ������ܹ����ӵĶ����������
		errorEmptyProc = -4,	///< ��������ĺ���ָ��Ϊ�գ�û������
	};

	/// ���ź�ģ���������ƥ��ĺ���ָ���������
	typedef FUNCTION_FUNCTION<void SIGNAL_TYPES_COMMA> Proc;

private:
	/// �źŽڵ�ṹ
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
	/// ���캯��
	/// \param maxSlots �ܹ����ӵ������ָ�����ĸ���
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

	 /// ��������
	~SIGNAL_SIGNAL()
	{
		delete []m_slots;
	}

	/// ���غ���ָ�����
	/// \param proc ����ָ�����
	/// \param position �������λ��
	/// \retval >=0 ���ú��Ѿ����ص��źŵĺ���ָ��������
	/// \retval <0 errorCode���͵Ĵ�����
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

	/// ж�غ���ָ����󣬸��ݶ����б���ĺ���ָ����ƥ�䡣
	/// \param proc ����ָ�����
	/// \param wait �Ƿ�ȴ����ڽ��еĻص�������һ����ʹ���߶���������ʱ����Ҫ�ȴ���
	///             ������ڻص�������ж�أ����ܵȴ����ȴ�Ҫ�ر�С�ģ���ֹ����.
	/// \retval >=0 ���ú��Ѿ����ص��źŵĺ���ָ��������
	/// \retval <0 errorCode���͵Ĵ�����
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
				/// �ص��̺߳�stop�̲߳���ͬһ�߳�ʱ������Ҫ�ȴ�������ȴ�����������
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

	/// �ж�ж�غ���ָ������Ƿ���أ����ݶ����б���ĺ���ָ����ƥ�䡣
	/// \param proc ����ָ�����
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

	/// ����()������������Ժ����������ʽ���������ӵ��źŵ����к���ָ�롣
	inline void operator()(SIGNAL_TYPE_ARGS)
	{
		CGuard guard(m_mutex);
		uint64_t us1 = 0, us2 = 0;

		m_threadId = CThread::getCurrentThreadID(); // ����ص��߳�ID

		for(int i = 0; i < m_numberMax; i++) // call back functions one by one
		{
			if(m_slots[i].state == slotStateNormal)
			{
				Proc temp = m_slots[i].proc;

				m_slots[i].running = true;
				m_mutex.leave();

				// ����ִ��������ͳ��
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