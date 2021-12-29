//
//  "$Id: MediaStreamSender.h 55559 2012-03-26 02:51:26Z zhou_mingwei $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-7-4		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_MEDIA_STREAM_SENDER_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_MEDIA_STREAM_SENDER_H__

#include "Infra/IntTypes.h"
#include "Infra/Function.h"
#include "Sock.h"
#include "NetHandler.h"
#include "Message.h"
#include "NDefs.h"
#include <string.h>

namespace Dahua{
namespace NetFramework{

class CMediaBuffer;
class CFrame;

//Ϊÿ��֡���弶���ܹ���5�����ڶ�֡ʱ������͵����ȶ�����
#define LEVEL_NUM		10 //5 -> 10, resolve "rtp lost packet issue between MTS -> SS";
//�����֡�ļ���LEVEL_HIGH������ߣ�LEVEL_LOW������ͣ��ܹ�LEVEL_NUM����
#define LEVEL_HIGH	1
#define	LEVEL_LOW		LEVEL_NUM

//��ʾý�����ݰ�������࣬���û�ʵ�����ݵĻ�ȡ���ͷš�
class NETFRAMEWORK_API CMediaPacket
{
public:
	CMediaPacket(){}
	virtual ~CMediaPacket(){}
public:
	//�ͷŸ����ݰ���
	virtual void Release() = 0;
	//��ȡ�����͵�ý�����ݰ���������ָ�롣
	virtual unsigned char *GetBuffer() = 0;
};

//��ý��ķ����ߣ�������һ��socket����CSockStream��ֻҪ��������һ������
//���뵽һ����ý�建����CMediaBuffer�ϣ�������ͨ��socket�ѻ������ϵ���
//���ݷ��ͳ�ȥ��
//ע�⣺ CMediaStreamSender����ͨ���侲̬��ԱCreate���������������������޷�����������
class NETFRAMEWORK_API CMediaStreamSender : public CNetHandler
{
	CMediaStreamSender( CMediaStreamSender const & other );
	CMediaStreamSender& operator=( CMediaStreamSender const & other );
public:
	//����AttachSendProcҪ�󶨵ĺ�������
	//������ void          ��������ֵ
	//			 CMediaPacket* ֡��Ϣ��ָ��
 	//			 int           ֡��Ϣ����
 	//       int           ֡����
	typedef Infra::TFunction3<void,CMediaPacket *,int, int > Proc;
	//���������ķ���ý������.
	//������ data: ���ݻ�������len: ���ݵĳ��ȡ�
	void SendOtherData( const char* data, int len );
	//��ȡ��ǰ�Ѿ�����һ�룬����δ���������İ��Ĳ������֡�
	//������buf��max_len�����ڽ��ոò������ֵĵĻ����������С��
	//����ֵ�����ڵ���0��ʾ�������Ĵ�С��С��0��ʾʧ�ܡ�
	//ע�⣺���øýӿں�CMediaStreamSender��Ϊ�ò������Ѿ����ͳ�ȥ���´β����ٷ��͡�
	int GetTailPacket( const char* buf, int max_len );
	//����һ��CMediaStreamSender����
	//����ֵ�� �ɹ��򷵻�һ��CMediaStreamSender���󣬷��򷵻�NULL��
	static CMediaStreamSender * Create();
	//������ͨ������ӿڣ�����CMediaStreamSender�����socket�����쳣������Ҫ
	//������߷�����ϢMSG_SOCK_EXCEPTION��
	//������ receiver_id: ��Ϣ�Ľ����ߣ�ͨ�����ǵ����ߡ�
	void WaitException( long receiver_id );
	//��һ��tcp socket�������ݽ������socket�Ϸ��͡���֮��
	//�Ͳ������������ط��ٶ����socket�������ݷ��Ͳ��������������������������������С�
	//������ stream: ���󶨵�stream����
	//����ֵ��0�ɹ���-1ʧ�ܡ�
	int Attach( CSock * stream );
	//���һ��tcp socket���󣬽��֮��CStreamSenderʧȥ�������ݵ�������
	//����ֵ������󶨵�stream����
	CSock * Detach();
	//��һ���ص�����,�������ͳɹ���֡����Ϣ
	//�����ظ��󶨣����һ�ΰ󶨵ĺ���Ϊ������
	//�������ͨ����������&func
	//����:Proc      typedef Infra::TFunction3<void,CMediaPacket *,int, int > Proc
	//����ֵ: true �ɹ���false ʧ��
	//ע��:����󶨵�Ϊ��Ա��������ʹ�÷�������
	//void classname::func(CMediaPacket*,int,int);
	//Proc proc(&classname::func,&object);
	//AttachSendProc(proc);
	bool AttachSendProc(Proc proc);
	//���һ��proc����
	//����ֵ:  true
	bool DetachSendProc();
	//���������������ӵ����������ԭ�����޷������ֽ������ʱ�䣬�����ڴ�ʱ�䣬�������쳣����
	//������sec������ĳ�ʱʱ�䣬��λΪ��
	void SetSendTimeout( unsigned int sec );
	//���÷��͵�ͨ���ţ�Ĭ������·�������ͨ��������
	//���Ե��ô˽ӿڽ��ж�����ã���ͬʱ֧�ַ��Ͷ�ͨ�����ݣ�����-1Ϊ�����Ĭ��
	//������channel��ͨ���ţ�ֻ֧��0-7
	void SetSendChannel( const int channel );
	//��ʹ�ܷ���ͨ������
	void DisableSendChannel( const int channel );
private:
	struct Internal;
	struct Internal*	m_internal;
};

//��ý�����ݶ����������Ľӿڶ��塣�û�ͨ���̳������ʵ���Լ�����ý�建������
//����������������뵽CMediaBuffer�У�Put�ӿھͻᱻ���ã��ͻ�õ���Ҫ����
//ý�����ݡ�
//ע�⣺�û�ʵ�ֵ������ͨ��new�����������ټ��뵽CMediaBuffer�С�
class NETFRAMEWORK_API CLevel2Buffer_IF
{
public:
	CLevel2Buffer_IF(): m_next(NULL){}
	virtual ~CLevel2Buffer_IF(){}
public:
	//���������������һ����ý�����ݰ�������ӿ���CMediaBuffer���ã��û����Լ�ʵ�֡�
	//������ packet: ����������ݰ���
	//			 len: ���ݰ��ĳ��ȡ�
	//			 mark: ���λ������ǰ����֡�����һ����mark==1������, mark==0��
	//			 level: ֡����Ҫ�ȼ�
	//����ֵ�� 0��ʾ�������ݰ���-1��ʾ����ʧ�ܣ�CMediaBuffer��ֱ��ͨ��deleteɾ���û�������
	virtual int Put( CMediaPacket* packet, int len, int mark, int level ) = 0;
private:
	friend class CMediaBuffer;
	CLevel2Buffer_IF* m_next;
};

//��ý�����ݻ�������
class NETFRAMEWORK_API CMediaBuffer{
	CMediaBuffer& operator=( CMediaBuffer const & other );
	CMediaBuffer( CMediaBuffer const & other );
public:
	CMediaBuffer();
	virtual ~CMediaBuffer();
public:
	//���һ����ý�巢����,����֮�󣬸÷����߾ʹӻ�������ʵʱ��λ�ÿ�ʼ
	//������ý�����ݡ�
	//������ Sender: �������ý�巢���ߡ�
	void AddSender( CMediaStreamSender * Sender );
	//�ӻ�������ɾ��һ����ý�巢���ߡ�
	//������ Sender: Ҫɾ������ý�巢���ߡ�
	void DelSender( CMediaStreamSender * Sender );
	//���һ����ý�����������,
	//������ buffer: �������ý�������������
	void AddLevel2Buffer( CLevel2Buffer_IF * buffer );
	//�ӻ�������ɾ��һ����ý�������������
	//������ Sender: Ҫɾ������ý�������������
	void DelLevel2Buffer( CLevel2Buffer_IF * buffer );
	//�򻺳�������һ����ý�����ݰ���
	//������ packet: ����������ݰ���
	//			 len: ���ݰ��ĳ��ȡ�
	//			 mark: ���λ������ǰ����֡�����һ����mark==1������, mark==0��
	//			 level: ֡����Ҫ�ȼ�����Χ��LEVEL_HIGH��LEVEL_LOW��ʹ����һ��Ҫ
	//							��ϸ�������ȼ�������LEVEL_HIGH��LEVEL_LOW�ķ�Χ�ڴ��¾��ȷֲ���
	//							����Ӱ�쵽���緢������ʱ�Ķ�֡���ָ���������
	//                          ��8bits����Я��ͨ����Ϣ��0-7����
	//                          ������CMediaStreamSender��SetSendChannel�ӿڽ���ͨ�����ݹ���
	//����ֵ�� 0��
	int Put( CMediaPacket* packet, int len, int mark, int level );
	//�򻺳�������һ��֡��Ϣ��
	//����:frameinfo:�ȴ������֡��Ϣ��
	//		:len:���ݰ��ĳ���
	//����ֵ:0.
	int PutFrameInfo(CMediaPacket * frameinfo,int len);
	//���֮ǰPut��MediaBuffer����δ���ͳ�ȥ������
	void Clear();
	//ʹ���Զ����ͻ��ƣ����ܻ��Թ̶����ڷ������ݰ���������֡�ʽϵͣ��������ϴ�ʱʹ��
	void EnableAutoSend();
	//ʹ���Զ����ͻ��ƣ������ָ�����ڷ������ݰ���������֡�ʽϵͣ��������ϴ�ʱʹ��
	//����:period:�Զ��������ڣ���λ: ����
	void EnableAutoSend(uint64_t period);
	//�ر��Զ����ͻ���
	void DisableAutoSend();
	//��ȡ�����buffer����
	int GetBufferNum();
	// ��ȡ�����֡�ֽ���
	uint64_t GetBufferSize();
private:
	struct Internal;
	struct Internal*		m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_MEDIA_STREAM_SENDER_H__
