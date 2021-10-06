//
//  "$Id$"
//
//  Copyright (c)1992-2011, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INFRA3_RW_MUTEX_H__
#define __INFRA3_RW_MUTEX_H__

#include "Defs.h"


namespace Dahua{
namespace Infra{

struct RWMutexInternal;

/// \class CMutex
/// \brief ��ƽ̨��������
class INFRA_API CReadWriteMutex
{
	CReadWriteMutex(CReadWriteMutex const&);
	CReadWriteMutex& operator=(CReadWriteMutex const&);

public:
	/// ���캯�����ᴴ��ϵͳ������
	CReadWriteMutex();

	/// ����������������ϵͳ������
	~CReadWriteMutex();

	/// �����ٽ�����
	/// \return �����Ƿ�ɹ�
	bool enterReading();

	/// �����ٽ�����
	/// \return �����Ƿ�ɹ�
	bool enterWriting();

	/// �뿪�ٽ�����
	/// \return �����Ƿ�ɹ�
	bool leave();

private:
	RWMutexInternal* m_internal;
};

} // namespace Infra
} // namespace Dahua

#endif //__INFRA_RW_MUTEX_H__

//
// End of "$Id: Mutex.h 16503 2010-11-26 02:23:15Z wang_haifeng $"
//