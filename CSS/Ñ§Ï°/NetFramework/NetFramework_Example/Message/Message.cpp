#include "Message.h"

#include <stdio.h>
#include <errno.h>
#include "Infra/Thread.h"
int CQuery::StartQuery()
{
	for( int i = 0; i < 10; i ++ ){
		m_report[i] = new CReport(i);
		m_report[i]->SetDefaultRcer( GetID() );
		Notify( m_report[i]->GetID(), MSG_QUERY, i ); //向每一个Report发送Query.
	}
	return 0;
}

int CQuery::handle_message( long from_id, MSG_TYPE type, long attach )
{
	switch( type ){
		case MSG_REPORT:	//收到Report，则删除Report对象。
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
			if( attach == m_id ){	//来自CQuery的MSG_QUERY把id号为和附带信息。
				fprintf( stdout, "%d receive query!\n",  m_id );
			}else
				fprintf( stderr, "error!\n" );
			Notify( MSG_REPORT, m_id );		//向默认接收者发送报告，也把自己的id号作为附带信息。
			break;
		default:
			break;
	}
	return 0;
}

//对于一个简单对象来说，handle_close其实是可以不实现的。
//但如果对象是new出来的，不实现可能会造成内存泄漏。
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
	
	// 如果输入q 字符退出
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