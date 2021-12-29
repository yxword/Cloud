//
//  "$Id: NetHandler.h 30412 2011-07-21 05:57:44Z qin_fenglin $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-11		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_NET_HANDLER_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_NET_HANDLER_H__

#include "Infra/IntTypes.h"
#include "NDefs.h"
#include "Sock.h"

namespace Dahua{
namespace NetFramework{

typedef unsigned int MSG_TYPE;
typedef unsigned int SOCK_MASK;
enum{
	NONE_MASK = 0,					//���¼���
	READ_MASK = (1 << 0),
	WRITE_MASK = (1 << 1),
	EXCEPT_MASK = (1 << 2),
	TIMEOUT_MASK = (1 << 3),		//��ʱ�¼���
};

class NETFRAMEWORK_API CNetHandler
{
	CNetHandler& operator=( CNetHandler const & other );
	CNetHandler( CNetHandler const & other );
public:
	CNetHandler();
	//˽���̳߳���CNetHandler�����캯����
	//����prv_idxΪCNetHandler�������ڵ�˽���̳߳�������
	CNetHandler( int32_t prv_idx );
	virtual ~CNetHandler();
protected:
	//���߳�ע��socket��һ��socket����ͬʱע�����д�¼��ġ�
	int RegisterSock( CSock & socket, SOCK_MASK type, int timout = 0 );
	int UnregisterSock( CSock & socket, SOCK_MASK type );
	//ɾ��ע���socket
	int RemoveSock( CSock & socket );

	//����һ����ʱ������λΪ΢��(�����֮һ��)
	//����Ϊ��ʱ������һ����ʱʱ�䣬��λΪ�����֮һ�롣
	//����ֵΪ��ʱ����ID�š�
	long SetTimer( int64_t usec );
	//���ٶ�ʱ��������ΪSetTimer���ص�ֵ��
	void DestroyTimer( long id );

	//����һ��CNetHandler���󴫵��첽��Ϣ����Ϣ����˳��ȷ��
	//������receiver_idΪ�����ߵ�ID,typeΪ��Ϣ���͡�infoΪ������Ϣ��
	void Notify( long receiver_id, MSG_TYPE type, long info );

	//������ͨ��SetDefaultRcer���õĹ̶������ߣ������첽��Ϣ��������ʽָ�������ߡ�
	void Notify( MSG_TYPE type, long info );

	//���л�������һ��CNetHandler���󴫵���Ϣ���ȷ��͵���Ϣ���ȱ�������
	// \param[in] receiver_id �����ߵ�ID
	// \param[in] type ��Ϣ����
	// \param[in] info ������Ϣ
	// \param[in] flag ��������
	void NotifySerial( long receiver_id, MSG_TYPE type, long info, unsigned int flag = 0 );
	void NotifySerial( MSG_TYPE type, long info, unsigned int flag = 0);

	//�����Ϣ�Ľ������ڴ������Ϣʱ����Ҫ�����豸I/O��������Ϊ��ʱ�Ĳ�����������������ӿ�
	//��������ͬ�ϡ�
	void NotifyIO( long receiver_id, MSG_TYPE type, long info );
	void NotifyIO( MSG_TYPE type, long info );

	//�ڴ��������ͷš������̻߳���汾����������NetFramework���߳��е��á�
	void* TsMalloc( uint32_t size );
	void TsFree( void* ptr );
	//�ڴ��������ͷţ����Ƿ��̻߳���汾��ʹ�ø�Ϊ���κ��߳��ж����Ե��ã���Ч�������½���
	void* Malloc( uint32_t size );
	void Free( void* ptr );
public:
	//ͨ���ýӿڣ���������һ������Ϊ�������Ĭ����Ϣ�����ߡ�
	void SetDefaultRcer( long receiver_id );
	//���ض����ID��
	long	GetID();

public:
	//�����¼���Ӧ�Ļص������������������ĸ�socket�ϵ��¼�������Щ����������
	//ͨ��handle�����֡�
	//����-1��ȡ��socket�ϵĵȴ���ʱ���á�
	//����0:ά��ԭ�еĵȴ���ʱ���á�
	//���ش���0����ʾ��һ���ȴ���ʱʱ�䣨��λΪ�����֮һ�룩��
	virtual int handle_input( int handle ) { return -1; }
	virtual int handle_output( int handle ) { return -1; }
	//Ϊ��ȫ�����ǿ�ҽ��������������������ʵ�ָú����������п��ܵ�������ϵͳ����Ӧ����
	virtual int handle_exception( int handle ) { return -1; }

	virtual int handle_input_timeout( int handle ) { return -1; }
	virtual int handle_output_timeout( int handle ) { return -1; }
	virtual int handle_exception_timeout( int handle ) { return -1; }
	//��ʱ����ʱ�Ļص���������λΪSetTimer���ص�ֵ��
	//����-1�����ٸö�ʱ��.
	//����0��ά��ԭ�еĶ�ʱ������.
	//���ش���0��ֵ����ʾ��һ����ʱʱ�䡣
	virtual int64_t handle_timeout( long id ) { return -1; }
	//CNetHandler�����������������ֻ������������в�����delete thisָ�룬�����������⡣
	virtual int handle_close( CNetHandler* myself )	{ return -1; }
	//��Ϣ��������from����Ϣ�ķ����ߣ�type����Ϣ���ͣ�attach����Ϣ�ĸ�������.
	//��Ϣ�����ͺ͸��������ɷ����ߺͽ�����֮��Լ������ܲ����κ�Լ����
	virtual int handle_message( long from_id, MSG_TYPE type, long attach ) { return -1; }

	//���ٶ��󣬷��Ǽ̳���CNetHandler�Ķ��󣬱���ͨ������ӿ������Լ�����������CHanddler����
	//Ҳ������øýӿڣ�Ȼ���ڻص�����hanlde_close()�д�����ص��������ˡ�
	//���ú�ᴥ��handle_close���첽�ص������ڴ����ص�ʱ�䲻ȷ������Ҫ��Close֮�����NetHandler����
	virtual void Close();

private:
	struct Internal;
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_NET_HANDLER_H__
