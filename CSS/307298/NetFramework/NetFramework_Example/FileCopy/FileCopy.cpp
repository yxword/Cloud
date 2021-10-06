#include "FileCopy.h"
#include "NetFramework/Message.h"
#include "NetFramework/NFile.h"
#include <stdio.h>
#include <unistd.h>
#include "Infra/Thread.h"
#define COPY_BUFFER_SIZE 32*1024
#define READ_BUFFER_NUM	4
#define READ_BUFFER_SIZE 32*1024
#define WRITE_BUFFER_NUM	0
#define WRITE_BUFFER_SIZE 0

CFileCopier::CFileCopier()
	:m_file_write(NULL),
	 m_file_read(NULL),
	 m_read_len(0),
	 m_write_len(0)
{
	m_rw_buf = new char[COPY_BUFFER_SIZE];
}

CFileCopier::~CFileCopier()
{
}

int CFileCopier::handle_close( CNetHandler* myself )
{
	delete this;
	return 0;
}

int CFileCopier::StartCopy()
{
	m_file_read = CNFile::Open( GetID(), "../software.rar", "r");
	if( m_file_read == NULL )
		return -1;
	m_file_write = CNFile::Open( GetID(), "../mysoftware.rar", "w" );	
	if( m_file_write == NULL ){
		m_file_read->Close(), m_file_read = NULL;
		return -1;
	}	
	m_file_read->SetBufferSize( READ_BUFFER_SIZE, READ_BUFFER_NUM );
	m_file_write->SetBufferSize( WRITE_BUFFER_SIZE, WRITE_BUFFER_NUM );
	
	while( copy_read() && copy_write() );

	while( 1 ){
		if( m_file_read == NULL && m_file_write == NULL ){
			break;
		}else
			usleep(100);
	}
	if( m_rw_buf != NULL )
		delete [] m_rw_buf, m_rw_buf = NULL;
	CNetHandler::Close();

	return 0;
}

bool CFileCopier::copy_read()
{
	bool is_EOF;
	bool func_ret = true;
	m_copy_mutex.enter();
	if( m_file_read == NULL ){
		func_ret = false;
	}else if( m_file_read != NULL && m_read_len == 0 ){
		m_read_len = m_file_read->Read( m_rw_buf, COPY_BUFFER_SIZE, is_EOF );
		if( is_EOF == true ){
			printf( "reach the file end! success!\n" );
			m_file_read->Close(), m_file_read = NULL;
		}else if( m_read_len < COPY_BUFFER_SIZE )
			func_ret = false;
	}
	m_copy_mutex.leave();
	return func_ret;
}

bool CFileCopier::copy_write()
{
	bool func_ret = false;
	m_copy_mutex.enter();
	if( m_file_write != NULL && m_read_len > m_write_len ){
		int ret = m_file_write->Write( m_rw_buf + m_write_len, m_read_len - m_write_len );
		m_write_len += ret;
		if( m_read_len == m_write_len ){
			m_read_len = 0;
			m_write_len = 0;
			func_ret = true;
			if( m_file_read == NULL )
				m_file_write->Close(), m_file_write = NULL;
		}
	}else if( m_file_read == NULL && m_file_write != NULL ){
		m_file_write->Close(), m_file_write = NULL;
	}
	m_copy_mutex.leave();
	return func_ret;
}

int CFileCopier::handle_message( long from_id, MSG_TYPE type, long attach )
{
	switch(type){
		case MSG_FILE_WRITE:
			if( copy_write() )
				while( copy_read() && copy_write() );
			break;
		case MSG_FILE_READ:
			while( copy_read() && copy_write() );
			break;
		case MSG_FILE_EXCEPTION:
			printf( "MSG_FILE_EXCEPTION!\n" );
			break;
		default:
			break;
	}			
	return 0;
}

#if 0
#endif 

int StartFileCopyTest()
{
	CFileCopier* file_copier = new CFileCopier;
	file_copier->StartCopy();
	
	// å¦‚æžœè¾“å…¥q å­—ç¬¦é€€å‡?
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
	
	if (file_copier)
		file_copier->Close();
		
	Dahua::Infra::CThread::sleep(2 * 1000);
	
	return 0;
}