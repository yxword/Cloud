#include "FileClient.h"
// TODO : handle_message支持统计最大连接数；最大IOPS？传输过程中断？ 
// 监听标准输入？
// 上传文件覆盖？
// using namespace std;
// using namespace Dahua::NetFramework;

CFileClient::CFileClient()
    : m_stream( NULL )
    , m_is_close( false )
    , m_connection_timer( 0 )
{
}

CFileClient::~CFileClient()
{
    close();
}

int CFileClient::connectServer( const CSockAddr &remote )
{
    if( m_stream != NULL ) {
        cout << "m_stream is not NULL" << endl;
        return -1;
    }

    m_stream = new CSockStream();
    if( m_stream->Connect( remote ) < 0 ){
        cout << "Connect server failed" << endl;
    }

    RegisterSock( *m_stream, WRITE_MASK, 10000000 );

    return 0;
}

// 读事件回调：获取文件内容； todo WriteV
int CFileClient::handle_input( int handle )
{
    // if( handle == m_stream->GetHandle() ) 

    char file_head[BUFFER_SIZE] = {'\0'};
    int ret = m_stream->Recv( file_head, BUFFER_SIZE );
    if( ret < 0 ){
		if( errno == ECONNRESET ) cout << "disconnecting by server" << endl;
		close();
        return 0;
	}

	if( ret == 0 ) {
		cout << "recv nothing" << endl;
	}

    cout << "file_head: " << file_head;

    CStringBufferParse string_buffer;
    string_buffer.parseBuffer( file_head );
    cout << "recv buffer size: " << string_buffer.size() <<endl;
    cout << "filename: " << string_buffer.filename() <<endl;
    if( ret < string_buffer.size() ){
        cout << "error : recv buffer size < file_head size" << endl;
    }

    if( string_buffer.request() != UPLOAD ) {
        cout << "request from server is not UPLOAD, download failed" << endl;
        return 0;
    }


    download( string_buffer.filename(), string_buffer.fileSize() );

    return 0;
}

// 写事件回调
int CFileClient::handle_output( int handle )
{
    int status = m_stream->GetConnectStatus();
    if( status == CSockStream::STATUS_NOTCONNECTED ){
        cout << "Not Connected" << endl;
        return 0;
    } 
    else if( status == CSockStream::STATUS_ERROR ){
        cout << "Connect server failed!" << endl;
        return -1;
    }
    else{
        // cout << "Connect server successfully!" << endl;
        UnregisterSock( *m_stream, WRITE_MASK );
        RegisterSock( *m_stream, READ_MASK | EXCEPT_MASK );
        m_connection_timer = SetTimer( 10000000000 );
        return -1;
    }
}

// int CFileClient::handle_input_timeout( int handle )
// {

// }

int CFileClient::handle_output_timeout( int handle )
{
    cout << "Connect server timeout!" << endl;
    RemoveSock( *m_stream );
    delete m_stream;
    m_stream = NULL;

    return -1;
}

int64_t CFileClient::handle_timeout(  long id )
{
    // m_timer_id  暂定定时器超时关闭连接
    if( id == m_connection_timer ) {
        cout << "Connection too long, close" << endl;
        close();
    }

    // TODO : send < 0, server close(can't recv) ,do close()
    return -1;
}

int CFileClient::handle_exception( int handle )
{
    return 0;
}

int CFileClient::handle_close( CNetHandler* myself )
{
    // TODO
    return 0;
}

void CFileClient::uploadFile( string filename )
{
    cout << "begin to upload file " << filename << endl;
    int ret = 0;
    ifstream read_file;

    string file_path = "../File/client/" + filename;
    read_file.open( file_path.c_str() );
    if( read_file == NULL ){
        cout << "error : this file not exists, upload failed " << endl;
        return;
    }
	
	// calculate file size
	read_file.seekg( 0, ios::end );
	uint32_t want_write_len = read_file.tellg(); 
	read_file.seekg( 0, ios::beg );

    // Send file head first  //todo while
    CStringBufferCreate string_buffer;
    string_buffer.append( "UPLOAD" );
    string_buffer.append( "FILENAME:%s", filename.c_str() );
    string_buffer.append( "SIZE:%d", want_write_len );
    char* file_head = string_buffer.getBuffer();
    ret = m_stream->Send( file_head, FILE_HEAD_SIZE );
    cout << "file_head: " << file_head;
    cout << "send buffer size: " << string_buffer.size() <<endl;
    if( ret < string_buffer.size() ){
        cout << "error : send buffer size < file_head size" << endl;
    }

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
            cout << "\r";
            cout << 1.0f * write_len * 100 / want_write_len << "%" ;
            fflush(stdout);
			write_once_offset += ret;
			write_len += ret;
		}
		else{
            // cout << "send size = 0, waiting" << endl;
			CThread::sleep( 1000 ); // sleep 1ms
		}
	}

	read_file.close();
	delete[] buf;
	buf = NULL;
	cout << endl << "write over, write length : " << write_len << endl << endl; 
}

void CFileClient::downloadFile( string filename )
{
    // Send download file request
    CStringBufferCreate string_buffer;
    string_buffer.append( "DOWNLOAD" );
    string_buffer.append( "FILENAME:%s", filename.c_str() );
    char* file_head = string_buffer.getBuffer();
    int ret = m_stream->Send( file_head, FILE_HEAD_SIZE );
    cout << "file_head: " << file_head << endl;
    cout << "send buffer size: " << string_buffer.size() << endl;
    if( ret < string_buffer.size() ){
        cout << "error : send buffer size < file_head size" << endl;
    }
}

void CFileClient::download( string filename, uint64_t file_size )
{
    int ret = 0;
	uint32_t want_read_len = file_size;

    string file_path = "../File/client/" + filename;
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
            cout << ret << endl;
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

void CFileClient::close()
{
    // ======================== ?????? ========================
    if( m_is_close ) return;
    RemoveSock( *m_stream );
    if( m_stream != NULL ) delete m_stream;
    m_stream = NULL;
    DestroyTimer( m_connection_timer );
    m_is_close = true;
    Close();
}

