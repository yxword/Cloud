//
//  "$Id: StreamSender.h 30412 2011-07-21 05:57:44Z qin_fenglin $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-7-2		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_STREAM_SENDER_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_STREAM_SENDER_H__

#include "Infra/IntTypes.h"
#include "NetHandler.h"
#include "Sock.h"
#include "Message.h"
#include "NDefs.h"

namespace Dahua{
namespace NetFramework{

//��ͨ������ý�壩TCP�����ݷ����ߡ�
//ע�⣺CStreamSender����ͨ��ͨ���侲̬��ԱCreate���������������������޷�����������
//�ر�ע�⣺CStreamSender����Ȼ������Attach����socket���󣬵�socket�����������
//�������ⲿ�ĵ����ߣ������������Լ��Ĵ�����ʵ�����µ�handle_output������
//		int handle_output( int handle )
//		{
//				return CStreamSenderObj->handle_output( handle );
//		}
class NETFRAMEWORK_API CStreamSender : public CNetHandler
{
	CStreamSender& operator=( CStreamSender const& other );
	CStreamSender( CStreamSender const& other );
public:
	//����һ��CStreamSender����
	//����ֵ�� �ɹ��򷵻�һ��CStreamSender���󣬷��򷵻�NULL��
	static CStreamSender * Create();
	//������������ݣ���Щ������CStreamSender�����ͣ������߲��ع��ġ�
	//������ buf: �����͵����ݡ�
	//			 len: ���������ݵĳ��ȡ�
	//����ֵ��0�ɹ���-1ʧ�ܡ�
	int Put( const char * buf, uint32_t len );
	//��һ��tcp socket�������ݽ������socket�Ϸ��͡���֮��
	//�Ͳ������������ط��ٶ����socket�������ݷ��Ͳ��������������������������������С�
	//������ stream: ���󶨵�stream����
	//����ֵ��0�ɹ���-1ʧ�ܡ�
	int Attach( CNetHandler *owner, CSock * stream );
	//���һ��tcp socket���󣬽��֮��CStreamSenderʧȥ�������ݵ�������
	//����ֵ������󶨵�stream����
	CSock * Detach();
	//����CStreanSender�ķ��ͻ������Ĵ�С��ȱʡΪ256K��Ҳ��8��32K��С��
	//������ size_in_32K���������Ĵ�С����32KΪ��λ��1��ʾ32K��2��ʾ64K��
	//����ֵ��0�ɹ���-1ʧ�ܡ�
	int SetBufferSize( uint32_t size_in_32K );
	//��ȡCStreamSender�ķ��ͻ������Ĵ�С��
	//����ֵ����ǰ��������С����32KΪ��λ��
	uint32_t GetBufferSize();
	//���CStreamSender�Ļ�������
	//����ֵ��0
	int Clear();
	//������ͨ������ӿڣ�����CStreamSender��һ��Put������Ҫ�Ļ�������С��
	//��CStreamSender�Ļ������ﵽҪ��ʱ���ͻ�������߷�����Ϣ��
	// ������ receiver_id����Ϣ�Ľ����ߣ�ͨ�����ǵ����ߡ�
	//				size_in_byte: ��������Ҫ�ȴ��Ŀ��л������Ĵ�С�����ֽ�Ϊ��λ��
	//ע�⣺�����ж��������Ҫ�������Ϣ��
	void WaitBufferSize( long receiver_id, uint32_t size_in_byte );
	//������ͨ������ӿڣ����Ի���Ѿ�put��ȥ����������������Ϣ
	void WaitSendFinish( long receiver_id );
	//������ͨ������ӿڣ�����CStreamSender�����socket�����쳣������Ҫ
	//������߷�����Ϣ��
	//������ receiver_id: ��Ϣ�Ľ����ߣ�ͨ�����ǵ����ߡ�
	//ע�⣺ֻ����һ��������Ҫ�������Ϣ��
	void WaitException( long receiver_id );	
private:
	struct Internal;
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_STREAM_SENDER_H__
