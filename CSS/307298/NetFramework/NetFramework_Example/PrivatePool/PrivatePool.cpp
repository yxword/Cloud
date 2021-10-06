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
		Notify( m_handle1[i]->GetID(), MSG_QUERY, i ); //向每一个Report发送Query.
	}
	return 0;
}

int CMyHandler::handle_message( long from_id, MSG_TYPE type, long attach )
{
	switch( type ){
		case MSG_REPORT:	//收到Report，则删除Report对象。
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
			if( attach == m_id ){	//来自CMyHandler的MSG_QUERY把id号为和附带信息。
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
int CMyHandler1::handle_close( CNetHandler* myself )
{
	//delete this;	//如果对象是new出来的，可以这样处理。
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