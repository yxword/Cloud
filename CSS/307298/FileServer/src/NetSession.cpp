#include "NetSession.h"
// #include "StringBuffer.h"

CNetSession::CNetSession( long svrID, CSockStream* stream)
	: m_is_close( false )
	, m_svr_id( svrID )
	, m_timer_id( 0 )
	, m_stream( stream )
{
	RegisterSock( *m_stream, READ_MASK | EXCEPT_MASK );
	m_timer_id = SetTimer( 60000000 );
}

CNetSession::~CNetSession()
{
	if( m_stream != NULL )
		delete m_stream;
}

int CNetSession::handle_input( int handle )
{
	// TODO
	int ret = 0;
	cout << "NetSession handle input" << endl;
	char file_head[BUFFER_SIZE] = {'\0'};
	ret = m_stream->Recv( file_head, BUFFER_SIZE );	

	if( ret < 0 ){
		if( errno == ECONNRESET ) cout << "disconnecting by client" << endl;
		closeSession();
		return 0;
	}

	if( ret == 0 ) {
		cout << "recv nothing" << endl;
		return 0;
	}

	cout << "file_head: " << file_head;
	CStringBufferParse string_buffer;
	string_buffer.parseBuffer( file_head );
	cout << "recv buffer size: " << string_buffer.size() << endl;
	// cout << "string_buffer.request() : " << string_buffer.request() <<endl;
    // if( ret != string_buffer.size() ){
    //     cout << "xxxx" << endl;
    // }

	if( string_buffer.request() == UPLOAD ){
		handleUploadRequest( string_buffer.filename(), string_buffer.fileSize() );
	}
	else if( string_buffer.request() == DOWNLOAD ){
		handleDownloadRequest( string_buffer.filename() );
	}
	else {
		cout << "can't handle this input type, file_head : " << file_head << endl;
		cout << string_buffer.request() << endl;
	}
	return 0;
}

void CNetSession::handleUploadRequest( string filename, uint64_t file_size )
{
	cout << "handle Upload Request" << endl;
	int ret = 0;
	uint32_t want_read_len = file_size;

	string file_path = "../File/server/" + filename;
    ofstream write_file;
	write_file.open( file_path.c_str() ); // default ios::trunc, not append 

	uint32_t read_once_len = 512*8*2048; //1MB  512Byte * N      
	char* buf = new char[read_once_len];
	uint32_t read_len = 0;
	while( read_len < want_read_len ){
		ret = m_stream->Recv( buf, read_once_len );

		if( ret == -1 ){
			cout << "recv error, break " << endl;
			break;
		}

		if( ret > 0 ){
			read_len += ret;
			uint32_t left_read_len = want_read_len - read_len;
			read_once_len = read_once_len > left_read_len ? left_read_len : read_once_len;

			write_file.write( buf, ret ); //
		}
		else if( ret == 0 ){
			// cout << "recv size = 0, waiting" << endl;
			CThread::sleep( 1000 ); // sleep 1ms
		}
		
	}

	write_file.close();
	delete[] buf;
	buf = NULL;
	cout << "read over, read length : " << read_len << endl << endl;
    
    return;
}

void CNetSession::handleDownloadRequest( string filename )
{
	cout << "handle Download Request" << endl;
	int ret = 0;

	string file_path = "../File/server/" + filename;
    ifstream read_file;
    read_file.open( file_path.c_str() );
    if( read_file == NULL ){
        cout << "error : this file not exists, upload failed " << endl;
        return;
    }
	
	// calculate file size
	read_file.seekg( 0, ios::end );
	uint32_t want_write_len = read_file.tellg(); 
	read_file.seekg( 0, ios::beg );

	CStringBufferCreate string_buffer;
	string_buffer.append( "UPLOAD");
	string_buffer.append( "FILENAME:%s", filename.c_str() );
	string_buffer.append( "SIZE:%d", want_write_len );
	char* file_head = string_buffer.getBuffer();
    ret = m_stream->Send( file_head, FILE_HEAD_SIZE );
	cout << "file_head: " << file_head;
    cout << "send buffer size: " << string_buffer.size() <<endl;
    // if( ret < string_buffer.size() ) {
    //     cout << "file head send failed" << endl;
    // }

	// Send file
	uint32_t write_once_len = 512*8*2048; //1MB  512Byte * N      
	char* buf = new char[write_once_len];
	uint32_t write_once_offset = write_once_len;
	uint32_t write_len = 0;
	while( write_len < want_write_len ){
		if( write_once_len == write_once_offset ){
			write_once_offset = 0;
			uint32_t left_write_len = want_write_len - write_len;
			write_once_len = write_once_len > left_write_len ? left_write_len : write_once_len;

			read_file.read( buf, write_once_len );                
		}
        ret = m_stream->Send( buf + write_once_offset, write_once_len - write_once_offset );
		
		if( ret == -1 ){
            cout << "send error! break" << endl;
            break;
        }

		if( ret > 0 ){
			write_once_offset += ret;
			write_len += ret;
		}
		else{
            // cout << "send size = 0, waiting" << endl;
			CThread::sleep( 1000 ); // sleep 1ms
			//SLEEP_MS( 1 );
		}
	}

	read_file.close();
	delete[] buf;
	buf = NULL;
	cout << "write over, write length : " << write_len << endl << endl; 

}

int64_t CNetSession::handle_timeout( long id )
{
	std::cout << "Session handle_timeout" << std::endl;
	char buffer[4];
	// if client closed(can't recv), close this session
	if( m_stream->Send( buffer, 0 ) < 0)
		closeSession();

	return 0;
}

int CNetSession::handle_exception( int handle )
{
	closeSession();

	return 0;
}

int CNetSession::handle_close( CNetHandler* myself )
{
	Notify( m_svr_id, MSG_SESSION_CLOSE, m_stream->GetHandle() );

	return 0;
}

void CNetSession::closeSession()
{
	if( m_is_close )
		return ;

	RemoveSock( *m_stream );
	DestroyTimer( m_timer_id);
	Close();
	m_is_close = true;
}

// Session Manager
CNetSessionManager::CNetSessionManager()
{

}

CNetSessionManager::~CNetSessionManager()
{
	clear();
}

CNetSession* CNetSessionManager::createFactory( long svrID, CSockStream* stream )
{
	CNetSession* session = new CNetSession( svrID, stream );
	m_mutex.enter();
	m_sessions.insert( make_pair( stream->GetHandle(), session ) );
	m_mutex.leave();
	return session;

}

void CNetSessionManager::remove( int handle )
{
	m_mutex.enter();
	map<int, CNetSession*>::iterator it = m_sessions.find( handle );
	if( it != m_sessions.end() ) {
		delete it->second;
		m_sessions.erase( it );
	}
	m_mutex.leave();
}

void CNetSessionManager::clear()
{
	m_mutex.enter();
	for( map<int, CNetSession*>::iterator it = m_sessions.begin(); it != m_sessions.end(); it ++ ){
		delete it->second;
	}
	m_sessions.clear();
	m_mutex.leave();
}