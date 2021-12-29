#include "SyncDataMessage.h"
#include "MessageType.h"
#include "HALog.h"

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

DBHALOG_CLASSNAME( CSyncDataMessage );

class CSyncDataMessage::Internal
{
public:
	Internal( EFS::ProtoHeader* header )
	{
		m_header_ptr = header;

		m_current_pos = 0;
		m_deserialize_data = NULL;
		m_deserialize_data_len = 0;

		m_header_ptr->type = EFS::DS_PROTOCOL_REQ;
		m_header_ptr->cmd = HAFK_SYNC_DATA_MESSAGE;
		m_header_ptr->seq = 1;
		m_header_ptr->bodyLen = 0;
		m_cmd_type = 0;
		m_data = NULL;
		m_len = 0;
	}

	~Internal()
	{
		if( NULL != m_deserialize_data ){
			delete[] m_deserialize_data;
			m_deserialize_data = NULL;
			m_deserialize_data_len = 0;
		}
	}

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len )
	{
		if( NULL == buf || 0 == len )
			return false;
		uint32_t header_len = sizeof( EFS::ProtoHeader );
		//第一次传递进来的不足一个协议头
		if( 0 == m_current_pos && len < header_len )
			return false;
		//一条完整的协议内容已经填充满,不允许再次使用
		if( m_deserialize_data_len == m_current_pos && 0 != m_current_pos )
			return false;

		if( 0 == m_current_pos ){
			m_sn.setDeserialize( buf, len );
			EFS::ProtoHeader tmp_header;
			if( EFS_SUCCESS != m_sn.readBytes( &tmp_header, header_len ) ){
				return false;
			}
			if( !checkPacketHeader( &tmp_header ) )
				return false;
			m_deserialize_data_len = tmp_header.bodyLen + header_len;
			if( NULL != m_deserialize_data )
				delete[] m_deserialize_data;
			m_deserialize_data = new char[m_deserialize_data_len];
			assert( NULL != m_deserialize_data );
		}

		if( m_deserialize_data_len - m_current_pos < len )
			return false;
		memcpy( ( m_deserialize_data + m_current_pos ), buf, len );
		m_current_pos += len;

		if( m_deserialize_data_len == m_current_pos ){
			return deserialize();
		}
		else{
			return true;
		}
	}
	//获取序列化的数据
	const char* getData()
	{
		int32_t len = 0;
		return serialize() ? m_sn.getSerialize( len ) : NULL;
	}
	//获取序列化的数据的长度
	uint32_t getDataLen()
	{
		int32_t len = 0;
		return NULL != m_sn.getSerialize( len ) ? len : 0;
	}

	//打印包用于debug
	void printPacket() const
	{
		DBHALOG_INFO( "CHAMessage: type:%u, cmd:%u seq:%u bodyLength:%u\n",
			m_header_ptr->type,m_header_ptr->cmd,m_header_ptr->seq,m_header_ptr->bodyLen );
		DBHALOG_INFO( "CHAMessage task[%d]", m_cmd_type );
	}

	//重新设置序列号.
	void setSequence( uint32_t seq )
	{
		m_header_ptr->seq = seq;
	}

	void setPktType( int32_t type )
	{
		m_header_ptr->type = type;
	}

	bool setSyncMessage( char* msg, int32_t len )
	{
		m_data = msg;
		m_len = len;
		return true;
	}

	bool getSyncMessage( char** msg, int32_t& len )
	{
		len = m_len;
		*msg = m_data;
		return true;
	}

private:
	//对自身进行序列化.
	bool serialize()
	{
        m_header_ptr->bodyLen = 0;
		m_header_ptr->bodyLen += EFS::sizeOfPointer( m_data, m_len );
		int64_t total_len = sizeof( EFS::ProtoHeader )+ m_header_ptr->bodyLen;
		m_sn.setSerialize( total_len );

		if( EFS_SUCCESS != m_sn.writeBytes( m_header_ptr, sizeof( EFS::ProtoHeader ) ) ) return false;
		if( EFS_SUCCESS != m_sn.writeString( m_data, m_len ) ) return false;
		return true;
	}

	//对接收的序列化数据,进行反序列化.
	bool deserialize()
	{
		m_sn.setDeserialize( m_deserialize_data, m_deserialize_data_len );
		if( EFS_SUCCESS != m_sn.readBytes( m_header_ptr, sizeof( EFS::ProtoHeader ) ) ) return false;
		if( EFS_SUCCESS != m_sn.readData( ( const char** )&m_data, m_len ) ) return false;
		return true;
	}

	//append内部用于检测插入的数据的包头是否合法.
	bool checkPacketHeader( const EFS::ProtoHeader* header )
	{
		return ( m_header_ptr->type == header->type && m_header_ptr->cmd == header->cmd ) ? true : false;
	}

private:
	//消息内容.
	int32_t m_cmd_type;
	char* m_data;
	int64_t m_len;

private:
	//类使用的变量.
	EFS::ProtoHeader* m_header_ptr;
	//存储序列化数据和提供序列化和反序列化接口.
	EFS::Serialization m_sn;
	uint32_t m_current_pos; //记录反序列数据插入的位置.
	char* m_deserialize_data;
	uint32_t m_deserialize_data_len;
};


CSyncDataMessage::CSyncDataMessage()
{
	m_internal = new Internal( &m_header );
	assert( NULL != m_internal );
}

CSyncDataMessage::CSyncDataMessage( uint32_t seq )
{
	m_internal = new Internal( &m_header );
	assert( NULL != m_internal );
	m_internal->setSequence( seq );
}

CSyncDataMessage::~CSyncDataMessage()
{
	if( m_internal != NULL ){
		delete m_internal;
		m_internal = NULL;
	}
}

bool CSyncDataMessage::append( const char* buf, uint32_t len )
{
	return m_internal->append( buf, len );
}
//获取序列化的数据
const char* CSyncDataMessage::getData()
{
	return m_internal->getData();
}
//获取序列化的数据的长度
uint32_t CSyncDataMessage::getDataLen()
{
	return m_internal->getDataLen();
}
//打印包用于debug
void CSyncDataMessage::printPacket() const
{
	m_internal->printPacket();
}

bool CSyncDataMessage::setSyncMessage( char* msg, int32_t len )
{
	return m_internal->setSyncMessage( msg, len );
}

bool CSyncDataMessage::getSyncMessage( char** msg, int32_t& len )
{
	return m_internal->getSyncMessage( msg, len );
}

class CResSyncDataMessage::Internal
{
public:
	Internal( EFS::ProtoHeader* header )
	{
		m_is_error = 0;
		m_error_type = 0;

		m_header_ptr = header;

		m_current_pos = 0;
		m_deserialize_data = NULL;
		m_deserialize_data_len = 0;

		m_header_ptr->type = EFS::DS_PROTOCOL_RES;
		m_header_ptr->cmd = HAFK_SYNC_DATA_MESSAGE;
		m_header_ptr->seq = 1;
		m_header_ptr->bodyLen = 0;
	}

	~Internal()
	{
		if( NULL != m_deserialize_data ){
			delete[] m_deserialize_data;
			m_deserialize_data = NULL;
			m_deserialize_data_len = 0;
		}
	}

	//尾部插入方式增加原始数据( 序列化后的 ).
	bool append( const char* buf, uint32_t len )
	{
		if( NULL == buf || 0 == len )
			return false;
		uint32_t header_len = sizeof( EFS::ProtoHeader );
		//第一次传递进来的不足一个协议头.
		if( 0 == m_current_pos && len < header_len )
			return false;
		//一条完整的协议内容已经填充满,不允许再次使用.
		if( m_deserialize_data_len == m_current_pos && 0 != m_current_pos )
			return false;

		if( 0 == m_current_pos ){
			m_sn.setDeserialize( buf, len );
			EFS::ProtoHeader tmp_header;
			if( EFS_SUCCESS != m_sn.readBytes( &tmp_header, header_len ) )
				return false;
			if( !checkPacketHeader( &tmp_header ) )
				return false;
			m_deserialize_data_len = tmp_header.bodyLen + header_len;
			if( NULL != m_deserialize_data )
				delete[] m_deserialize_data;
			m_deserialize_data = new char[m_deserialize_data_len];
			assert( NULL != m_deserialize_data );
		}

		if( m_deserialize_data_len - m_current_pos < len )
			return false;
		memcpy( ( m_deserialize_data + m_current_pos ), buf, len );
		m_current_pos += len;

		if( m_deserialize_data_len == m_current_pos ){
			return deserialize();
		}
		else{
			return true;
		}
	}
	//获取序列化的数据.
	const char* getData()
	{
		int32_t len = 0;
		return serialize() ? m_sn.getSerialize( len ) : NULL;
	}
	//获取序列化的数据的长度.
	uint32_t getDataLen()
	{
		int32_t len = 0;
		return NULL != m_sn.getSerialize( len ) ? len : 0;
	}
	//打印包用于debug.
	void printPacket() const
	{
		DBHALOG_DEBUG( "CResSyncDataMessage: type:%u, cmd:%u seq:%u bodyLength:%u\n",
			m_header_ptr->type,m_header_ptr->cmd,m_header_ptr->seq,m_header_ptr->bodyLen );
		DBHALOG_DEBUG( "is error:%d,error type:%d\n",m_is_error,m_error_type );
	}
	//重新设置序列号.
	void setSequence( uint32_t seq )
	{
		m_header_ptr->seq = seq;
	}

	//协议内容.
	//包有错误,则设置错误类型进去,否则默认包是正确的.
	void setErrorType( int32_t errorType )
	{
		m_is_error = -1;
		m_error_type = errorType;
	}
	//获取包是否有错,0表示正确,-1标识错误.
	int8_t isError() const
	{
		return m_is_error;
	}
	//如果包出错,则获取到的错误类型标识是什么错误.
	int32_t getErrorType() const
	{
		return m_error_type;
	}

private:
	//对自身进行序列化.
	bool serialize()
	{
        m_header_ptr->bodyLen = 0;
        m_header_ptr->bodyLen += INT8_SIZE;
        m_header_ptr->bodyLen += INT32_SIZE;
		int64_t total_len = sizeof( EFS::ProtoHeader ) + m_header_ptr->bodyLen;
		m_sn.setSerialize( total_len );
		if( EFS_SUCCESS != m_sn.writeBytes( m_header_ptr, sizeof( EFS::ProtoHeader ) ) ) return false;
		if( EFS_SUCCESS != m_sn.writeInt8( m_is_error ) ) return false;
		if( EFS_SUCCESS != m_sn.writeInt32( m_error_type ) ) return false;
		return true;
	}

	//对接收的序列化数据,进行反序列化.
	bool deserialize()
	{
		m_sn.setDeserialize( m_deserialize_data, m_deserialize_data_len );
		if( EFS_SUCCESS != m_sn.readBytes( m_header_ptr, sizeof( EFS::ProtoHeader ) ) ) return false;
		if( EFS_SUCCESS != m_sn.readInt8( &m_is_error ) ) return false;
		if( EFS_SUCCESS != m_sn.readInt32( &m_error_type ) ) return false;
		return true;
	}

	//append内部用于检测插入的数据的包头是否合法.
	bool checkPacketHeader( const EFS::ProtoHeader* header )
	{
		return ( m_header_ptr->type == header->type && m_header_ptr->cmd == header->cmd ) ? true : false;
	}

private:
	int8_t m_is_error;
	int32_t m_error_type;

private:
	//类使用的变量.
	EFS::ProtoHeader* m_header_ptr;
	//存储序列化数据和提供序列化和反序列化接口.
	EFS::Serialization m_sn;
	uint32_t m_current_pos; //记录反序列数据插入的位置.
	char* m_deserialize_data;
	uint32_t m_deserialize_data_len;
};

CResSyncDataMessage::CResSyncDataMessage()
{
	m_internal = new Internal( &m_header );
	assert( NULL != m_internal );
}
CResSyncDataMessage::CResSyncDataMessage( uint32_t seq )
{
	m_internal = new Internal( &m_header );
	assert( NULL != m_internal );
	m_internal->setSequence( seq );
}
CResSyncDataMessage::~CResSyncDataMessage()
{
	if( NULL != m_internal )
		delete m_internal;
	m_internal = NULL;
}

//尾部插入方式增加原始数据( 序列化后的 ).
bool CResSyncDataMessage::append( const char* buf, uint32_t len )
{
	return m_internal->append( buf, len );
}
//获取序列化的数据.
const char* CResSyncDataMessage::getData()
{
	return m_internal->getData();
}
//获取序列化的数据的长度.
uint32_t CResSyncDataMessage::getDataLen()
{
	return m_internal->getDataLen();
}
//打印包用于debug.
void CResSyncDataMessage::printPacket() const
{
	m_internal->printPacket();
}

//协议内容.
//包有错误,则设置错误类型进去,否则默认包是正确的.
void CResSyncDataMessage::setErrorType( int32_t errorType )
{
	m_internal->setErrorType( errorType );
}
//获取包是否有错,0表示正确,-1标识错误.
int8_t CResSyncDataMessage::isError() const
{
	return m_internal->isError();
}
//如果包出错,则获取到的错误类型标识是什么错误.
int32_t CResSyncDataMessage::getErrorType() const
{
	return m_internal->getErrorType();
}

EFS::TPacketBasePtr newCSyncDataMessage()
{
	return EFS::TPacketBasePtr( new CSyncDataMessage() );
}

EFS::TPacketBasePtr newCResSyncDataMessage()
{
	return EFS::TPacketBasePtr( new CResSyncDataMessage() );
}

} // DBHA
} // VideoCloud
} // Dahua

