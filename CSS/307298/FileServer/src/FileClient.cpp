#include "FileClient.h"
// TODO : handle_message支持统计最大连接数；最大IOPS？传输过程中断？ 
// 监听标准输入？
// 上传文件覆盖？
using std::string;
using std::ifstream;
using std::ofstream;
using std::ios;
using namespace Dahua::NetFramework;

CFileClient::CFileClient()
    : m_is_close( false )
    , m_stream( NULL )
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
    // char buff[]
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
        cout << "Connect server successfully!" << endl;
        UnregisterSock( *m_stream, WRITE_MASK );
        RegisterSock( *m_stream, READ_MASK | EXCEPT_MASK );
        m_connection_timer = SetTimer( 10000000000 );
    }
}

int CFileClient::handle_input_timeout( int handle )
{

}

int CFileClient::handle_output_timeout( int handle )
{
    cout << "Connect server timeout!" << endl;
    RemoveSock( *m_stream );
    delete m_stream;
    m_stream = NULL;

    return -1;
}

int CFileClient::handle_timeout( int handle )
{
    // m_timer_id  暂定定时器超时关闭连接
    if( handle == m_connection_timer ) {
        cout << "Connection too long, close" << endl;
        close();
    }
    return -1;
}

int CFileClient::handle_exception( int handle )
{

}

int CFileClient::handle_close( CNetHandler* myself )
{
    // ======================== ?????? ========================
    RemoveSock( *m_stream );
    m_stream = NULL;
    DestroyTimer( m_connection_timer );
    m_is_close = true;
}

// void CFileClient::close()
// {
//     if( m_is_close ) return;

//     RemoveSock( *m_stream );
//     m_stream = NULL;
//     DestroyTimer( m_timer_id );
//     Close();
//     m_is_close = true;
// }

int CFileClient::downloadFile()
{

}

int CFileClient::uploadFile( string filename )
{
    ifstream read_file;

    read_file.open( filename.c_str() );
    if( read_file == NULL ){
        cout << "error : this file not exists, upload failed " << endl;
        return;
    }
	
    string request_buff = "uploadFile " + filename;
    int ret = m_stream->Send( request_buff.c_str(),  sizeof( request_buff ) );
    cout << "request_buff size, " << ret << endl;
    
	// calculate file size
	read_file.seekg( 0, ios::end );
	uint32_t want_write_len = read_file.tellg(); 
	read_file.seekg( 0, ios::beg );
	// uint32_t want_write_len = 300 << 20; // 300M
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
		//int ret = file.write( buf + write_once_offset, write_once_len - write_once_offset );
		// TODO send or write buffer
        ret = 

		// int ret = Write(...);
		
		if( ret > 0 ){
			write_once_offset += ret;
			write_len += ret;
		}
		else if( ret == 0 ){
			//SLEEP_MS( 1 );
		}
		else if ( ret == -1 ){
			//cout << "write failed, error : " << Dahua::EFS::getLastError() << endl;
			cout << "write error " << endl;
			break;
		}
		else {
			// assert ( 0 );
		}
	}

	read_file.close();
	delete[] buf;
	buf = NULL;
	cout << "write over, write length : " << write_len << endl; 
}

void CFileClient::close()
{
    // ======================== ?????? ========================
    if( m_is_close ) return;
    Close();
}

