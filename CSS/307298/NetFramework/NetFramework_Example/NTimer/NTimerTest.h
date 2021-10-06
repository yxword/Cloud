//
//  "$Id: Timer.h 7672 2010-5-17 02:28:18Z he_linqiang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-5-17        he_linqiang Create
//
//
#ifndef __INCLUDED_NTIMER_TEST_H__
#define __INCLUDED_NTIMER_TEST_H__

#include "NetFramework/NetHandler.h"
using namespace Dahua::NetFramework;


///\brief �Ự����Ķ�ʱ��(10�뼶)�����Ự����ʹ��
class CAliveTimer : public NetFramework::CNTimerEvent
{
public:
	static CAliveTimer* create();
	
	///\brief ������ʱ��
	int startAliveTimer();
	
	///\brief ֹͣ��ʱ��
	int stopAliveTimer();
	
	///\brief �ͷŶ�ʱ����Դ
	void destroy();
	
private:
	///\brief ���캯��
	CAliveTimer();
	
	///\brief ��������
	~CAliveTimer();

	///\brief ����ӿڣ���������NTimerʹ��
	int handle_timer_event();	

	///\brief �����ܻ���ӿ�
	int handle_close( NetFramework::CNetHandler* myself );
private:
	int					m_ref_count;						///< ���ڶ�ʱ����ʱ����
	static NetFramework::CNTimer	*m_static_alivetimer;	///< ȫ�ֶ�ʱ��
	static Infra::CMutex			m_static_alivemutex;							///< ��ʱ��������id,  ��ʱ���������id ������Ϣ
};


#endif //__INCLUDED_NTIMER_TEST_H__
