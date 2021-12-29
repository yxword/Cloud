//
//  "$Id: NetThreadExtra.h 281999 2015-07-14 06:14:47Z shu_wang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-11		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_NET_THREAD_EXTRA_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_NET_THREAD_EXTRA_H__
#include "NDefs.h"

/*------------------֣������-----------------------*/
/*���½ӿ�����ʹ�ã���������������ܿ⸺�������ۣ�
���������֤�Ժ󷽿�ʹ�á����ޱ�Ҫ,������ʹ��!!!*/


#ifdef __cplusplus
extern "C" {
#endif

namespace Dahua{
namespace NetFramework{

	//������ý������߳����ȼ����ú�����CreateThreadPool�ӿ�ǰ���á�
	//������
	//		priority 	�߳����ȼ���ֵԽС��ʾ���ȼ�Խ�ߣ��ᱻת���ɶ�Ӧ����ϵͳ
	//	     		  	ƽ̨�����ȼ���ȡֵ1-127��
	// 	policy   	�̵߳��Ȳ���: 0, ��ͨ�߳�; 1, ʵʱ�߳�
	// 	stackSize 	Ϊ�߳�ָ���Ķ�ջ��С���������0����С��ƽ̨Ҫ������ֵ��
	//        			��ʹ��ƽ̨ȱʡֵ��
	//����ֵ��-1ʧ�� 0�ɹ�
int NETFRAMEWORK_API SetRecvThreadPriority( int priority, int policy = 0, int stack_Size = 0 );

}
}

#ifdef __cplusplus
}
#endif

#endif //__INCLUDED_DAHUA_NETFRAMEWORK_NET_THREAD_H__
