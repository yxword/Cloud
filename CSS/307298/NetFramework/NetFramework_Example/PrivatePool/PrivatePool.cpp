#include "PrivatePool.h"
#include "NetFramework/NetThread.h"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

void CPool::Start()
{
	printf("create public thread pool!\n");
	if( CNetThread::CreateThreadPool( 4 ) < 0 ){
		fprintf( stderr, "create public thread pool fail!\n" );
		CNetThread::DestroyThreadPool();
	}
	
	printf("create private thread pool!\n");
	int pri_idx1 = 0;
	if( (pri_idx1 = CNetThread::CreatePrivatePool( 4 )) < 0 ){
		fprintf( stderr, "create private thread pool fail!\n" );
		CNetThread::DestroyPrivatePool(pri_idx1);
	}
	my_handler = new CMyHandler;
	my_handler->StartQuery( pri_idx1 );
	sleep(1);
	delete my_handler, my_handler = NULL;
	CNetThread::DestroyThreadPool();
	CNetThread::DestroyPrivatePool(pri_idx1);
}

int CMyHandler::StartQuery( int idx )
{
	for( int i = 0; i < 10; i ++ ){
		m_handle1[i] = new CMyHandler1( idx, i );
		m_handle1[i]->SetDefaultRcer( GetID() );
		Notify( m_handle1[i]->GetID(), MSG_QUERY, i ); //��ÿһ��Report����Query.
	}
	return 0;
}

int CMyHandler::handle_message( long from_id, MSG_TYPE type, long attach )
{
	switch( type ){
		case MSG_REPORT:	//�յ�Report����ɾ��Report����
			fprintf( stdout, "from %ld report %ld correct!\n", from_id, attach );
			if( (attach >= 0 && attach < 10) && m_handle1[attach] != NULL ){ 
				m_handle1[attach]->Close();
				m_handle1[attach]  = NULL;
			}
			break;
		default:
			break;
	}
	return 0;
}

CMyHandler1::CMyHandler1( int pool_idx, int id ):CNetHandler( pool_idx ), m_id( id )
{
}

int CMyHandler1::handle_message( long from_id, MSG_TYPE type, long attach )
{
	switch( type ){
		case MSG_QUERY:
			if( attach == m_id ){	//����CMyHandler��MSG_QUERY��id��Ϊ�͸�����Ϣ��
				fprintf( stdout, "%d receive query!\n",  m_id );
			}else
				fprintf( stderr, "error!\n" );
			Notify( MSG_REPORT, m_id );		//��Ĭ�Ͻ����߷��ͱ��棬Ҳ���Լ���id����Ϊ������Ϣ��
			break;
		default:
			break;
	}
	return 0;
}

//����һ���򵥶�����˵��handle_close��ʵ�ǿ��Բ�ʵ�ֵġ�
//�����������new�����ģ���ʵ�ֿ��ܻ�����ڴ�й©��
int CMyHandler1::handle_close( CNetHandler* myself )
{
	//delete this;	//���������new�����ģ�������������
	return -1;
}



int StartPrivatePoolTest()
{
	CPool pool;
	pool.Start();
	
	char c = 0;
	while(c = getchar())
	{
		if ('q' == c)
		{
			DLOG_INFO("exit \n");
			break;
		}

		Infra::CThread::sleep(1 * 1000);
	}
	
	Infra::CThread::sleep(2 * 1000);
	
	return 0;
}