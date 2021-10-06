#include "StringBuffer.h"

CStringBufferCreate::CStringBufferCreate()
{
    memset( m_string_buffer, '\0', BUFFER_SIZE );
    m_index = 0;
}

bool CStringBufferCreate::append( const char* format, ... )
{
    if( m_index >= BUFFER_SIZE ) return false;

    va_list arg_list;
    va_start( arg_list, format );

    int len = vsnprintf( m_string_buffer + m_index, BUFFER_SIZE - m_index - 1, format, arg_list );
    if( len >= BUFFER_SIZE - m_index - 1 ) {
        va_end( arg_list );
        return false;
    }

    m_index += len;
    va_end( arg_list );

    len = snprintf( m_string_buffer + m_index, 3, "\r\n");  // "\r\n\0"
    m_index += len;

    return true;
}

char* CStringBufferCreate::getBuffer()
{
    return m_string_buffer;
}

CStringBufferParse::CStringBufferParse()
    : m_string_buffer( NULL )
    , m_index( 0 )
    , m_filename()
    , m_size( 0 )
    , m_parse_request( true )
    , m_request( UNKNOWN )
{

}

CStringBufferParse::~CStringBufferParse()
{
    m_string_buffer = NULL;
}

// 处理每一行，\r\n 结尾代表一行
// 将\r\n 替换成 \0\0
bool CStringBufferParse::parseLine( char* buffer ){
    int i = 0;
    for( ; i < BUFFER_SIZE - m_index; i++){
        // if( buffer[i] == '\0' ) break;
        if( buffer[i] == '\r' && buffer[i + 1] == '\n'){
            buffer[i] = '\0';
            buffer[i+1] = '\0';
            m_index += i + 2;
            return true;
        }
    }
    return false;
}

bool CStringBufferParse::parseBuffer( char* buffer )
{
    m_index = 0;
    m_string_buffer = buffer;
    m_filename = "";
    m_size = 0;
    m_parse_request = true;
    m_request = UNKNOWN;

    // cout << "begin to parse " << endl;
    // cout << "buffer: " << buffer << endl;

    if( m_string_buffer == NULL ) {
        cout << "Buffer is NULL" <<endl;
        return false;
    }
    char* tmpBuffer = m_string_buffer;
    while( parseLine( tmpBuffer ) ){       
        // cout << tmpBuffer << endl;
        // cout << m_index << endl; 
        if( tmpBuffer[0] == '\0' ){
            if( m_index == 0 ) cout << "Buffer has no content" << endl;
            return false;
        }

        if( m_parse_request ) {
            parseRequestLine( tmpBuffer );
            m_parse_request = false;
        }
        else if( strncasecmp( tmpBuffer, "FILENAME:", 9 ) == 0 ){
            tmpBuffer += 9;
            // tmpBuffer += strspn( tmpBuffer, "\t" );
            m_filename = tmpBuffer;
        }
        else if( strncasecmp( tmpBuffer, "SIZE:", 5 ) == 0 ){
            tmpBuffer += 5;
            m_size = atoi( tmpBuffer );
        }
        
        tmpBuffer = m_string_buffer + m_index;
    }
    return true;
}

void CStringBufferParse::parseRequestLine( char* buffer )
{
    if( strcasecmp( buffer, "UPLOAD" ) == 0 ) {
        m_request = UPLOAD;
    }
    else if( strcasecmp( buffer, "DOWNLOAD" ) == 0 ) {
        m_request = DOWNLOAD;
    }
}