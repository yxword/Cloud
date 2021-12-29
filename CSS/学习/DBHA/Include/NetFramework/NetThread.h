//
//  "$Id: NetThread.h 26645 2011-05-31 00:35:01Z wang_haifeng $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-11		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_NET_THREAD_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_NET_THREAD_H__

#include "Infra/IntTypes.h"
#include "NDefs.h"






namespace Dahua{
namespace NetFramework{

//�����������ģ�͵����ĵ���ϵͳ
class CNetThread
{
public:
	enum Priority
	{
		priorTop = 1,
		priorBottom = 127,
		priorDefault = 64,
	};
	enum Policy
	{
		policyNormal = 0,  ///< ��ͨ�߳�
		policyRealtime = 1 ///< ʵʱ�߳�
	};
	enum{
		HIGH_REALTIME = 0,
		REALTIME = 1,
		BALANCE = 2,
		PERFORMANCE = 3
	};
public:
	//����һ���������̡߳��ú�����ϵͳ��ʼ��ʱ����
	//������thread_numΪ�������������е��߳�������
	//		use_aioΪ�Ƿ�ʹ���첽����IO������true������Ҫʹ��CNFile�����д����첽IO����
	//		����CNFile��֧��ͬ������
	//����ֵ��-1ʧ�� 0�ɹ�
	static int NETFRAMEWORK_API CreateThreadPool( uint32_t thread_num, bool use_aio = false );
	//����˽���̳߳�
	//������thread_numΪ˽���̳߳������е��߳�������
	//		use_aioΪ�Ƿ�ʹ���첽����IO������true������Ҫʹ��CNFile�����д����첽IO����
	//		����CNFile��֧��ͬ������
	//����ֵ��-1������˽���̳߳�ʧ��
	//		>0, ����˽���̳߳سɹ�������ֵΪ˽���̳߳������ţ�����˽���̳߳��е�NetHandler������Ҫ
	//		ʹ�ø�����ֵ���Ա�ʶNetHandler���������ĸ�˽�г�
	static int NETFRAMEWORK_API CreatePrivatePool( uint32_t thread_num, bool use_aio = false );
	//�����������ڽ�����ý������ʱ�Ľ��ղ��ԣ�ע�⣺����ڲ��Ѿ�Ϊ��ý����������˺���Ĳ��ԣ�
	//��û��������Ҫ���벻Ҫ������øýӿڡ��ú�����CreateThreadPool�ӿ�ǰ���á�
	//������level��ָ���Ĳ��ԣ���ѡ�Ĳ����У�
	//						HIGH_REALTIME����ʵʱ�ԡ��ò����£������ܿ�ؽ��������ϵ����ݣ�����������Ϊ���۱�֤������������
	//						REALTIME: ʵʱ�ԣ��ò����£���֤���󲿷�����£����ӳٵؽ��������ϵ���ý�����ݡ�ΪĬ�ϲ��ԡ�
	//						BALANCE��ƽ�⣬�ò��Լ�����������ݵ�ʵʱ�ԣ���Ӳ���������ã���ʵʱ��Ҫ�󲻸ߵ�����¿�ѡ�ò��ԡ�
	//						PERFORMANCE���������ȣ�������ʵʱ��Ϊ���۱�֤���ܡ�
	static void NETFRAMEWORK_API SetRcvPriority( int level );
	//�����������߳����ȼ����ú�����CreateThreadPool�ӿ�ǰ���á�
	//������
	//		priority 	�߳����ȼ���ֵԽС��ʾ���ȼ�Խ�ߣ��ᱻת���ɶ�Ӧ����ϵͳ
	//	     		  	ƽ̨�����ȼ���ȡֵpriorTop��priorBottom��Ĭ��ֵpriorDefault��
	/// 	policy   	�̵߳��Ȳ���
	/// 	stackSize 	Ϊ�߳�ָ���Ķ�ջ��С���������0����С��ƽ̨Ҫ������ֵ��
	///        			��ʹ��ƽ̨ȱʡֵ��
	//����ֵ��-1ʧ�� 0�ɹ�
	static int NETFRAMEWORK_API SetThreadPriority( int priority, int policy = policyNormal, int stackSize = 0 );

	//���ٹ����̳߳ء�
	//����ֵ��0�ɹ� ��ʧ�������
	static int NETFRAMEWORK_API DestroyThreadPool();
	//��������Ϊidx��˽���̳߳ء�
	//������idx˽���̳߳�������Ϊ��������˽���̳߳�ʱ�ķ���ֵ��
	//����ֵ��0�ɹ� ��ʧ�������
	static int NETFRAMEWORK_API DestroyPrivatePool( int32_t idx );
	//��������ӡע�ᵽNEtFramework�е�����socket��fd,����CNetHandler����ָ�뼰��������Ϣ��
	//��������
	//����ֵ��ע��NEtFramework�е�����socket��������
	static int NETFRAMEWORK_API TraversalSock();
	//����R3 ���� �ڴ����̳߳�֮��/ �����̳߳�֮ǰ����
	//��������
	//����ֵ��0�������ɹ���-1������ʧ��
	static int NETFRAMEWORK_API EnableR3();
	//����R3 �����ڴ����̳߳�֮��/ �����̳߳�֮ǰ����
	//��������
	//����ֵ��0���رճɹ�����ʧ�����
	static int NETFRAMEWORK_API DisableR3();
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_NET_THREAD_H__
