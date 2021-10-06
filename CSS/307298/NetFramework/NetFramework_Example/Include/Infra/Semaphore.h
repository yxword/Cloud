//
//  "$Id: Semaphore.h 55347 2012-03-22 11:26:45Z qin_fenglin $"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INFRA3_SEMAPHORE_H__
#define __INFRA3_SEMAPHORE_H__

#include "Defs.h"

#if defined(__linux__)
#include <semaphore.h>
#endif

namespace Dahua{
namespace Infra{

struct SemaphoreInternal;

/// \class CSemaphore
/// \brief ��ƽ̨�ź�����
class INFRA_API CSemaphore
{
	CSemaphore(CSemaphore const&);
	CSemaphore& operator=(CSemaphore const&);

public:
	/// ���캯�����ᴴ��ϵͳ�ź���
	/// \param initialCount �ź�����ʼ����
	explicit CSemaphore(int initialCount = 0);

	/// ����������������ϵͳ������
	~CSemaphore();

	/// �����ź�������������Ѿ����ٵ�0�����������õ��߳�
	/// \return ��ǰ�ź�������
	int pend();

	/// �����ź�������������Ǵ�0�ۼӣ��ỽ����ȴ����еĵ�һ���߳�
	/// \return ��ǰ�ź�������
	int post();

private:
	SemaphoreInternal* m_internal;
};

} // namespace Infra
} // namespace Dahua

#endif //__INFRA_SEMAPHORE_H__
//
// End of "$Id: Semaphore.h 55347 2012-03-22 11:26:45Z qin_fenglin $"
//
