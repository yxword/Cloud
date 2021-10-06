#include "Message.h"

#include <stdio.h>
#include <errno.h>
#include "Infra/Thread.h"
int CQuery::StartQuery()
{
	for( int i = 0; i < 10; i ++ ){
		m_report[i] = new CReport(i);
		m_report[i]->SetDefaultRcer( GetID() );
		Notify( m_report[i]->GetID(), MSG_QUERY, i ); //��ÿһ��Report����Query.
	}
	return 0;
}

int CQuery::handle_message( long from_id, MSG_TYPE type, long attach )
{
	switch( type ){
		case MSG_REPORT:	//�յ�Report����ɾ��Report����
			fprintf( stdout, "from %ld report %ld correct!\n", from_id, attach );
			if( (attach >= 0 && attach < 10) && m_report[attach] != NULL ){ 
				m_report[attach]->Close();
				m_report[attach]  = NULL;
			}
			break;
		default:
			break;
	}
	return 0;
}

int CReport::handle_message( long from_id, MSG_TYPE type, long attach )
{
	switch( type ){
		case MSG_QUERY:
			if( attach == m_id ){	//����CQuery��MSG_QUERY��id��Ϊ�͸�����Ϣ��
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
int CReport::handle_close(CNetHandler *myself)
{
	delete myself;
}

#if 0
#endif 

int StartMessageTest()
{	
	CQuery *query = new CQuery;
	if( query->StartQuery() < 0 )
		return -1;
	
	// �������q �ַ��˳�
	char c = 0;
	while(c = getchar())
	{
		if ('q' == c)
		{
			printf("exit \n");
			break;
		}

		Dahua::Infra::CThread::sleep(1 * 1000);
	}
	
	if (query)
		query->Close();
		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}