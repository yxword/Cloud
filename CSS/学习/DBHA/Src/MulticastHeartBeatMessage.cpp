


#include "MulticastHeartBeatMessage.h"
#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"
#include "Infra/PrintLog.h"
#include "MessageType.h"
#include "HALog.h"


namespace Dahua {
namespace VideoCloud {
namespace DBHA {

using namespace Infra;
using namespace EFS;

DBHALOG_CLASSNAME(CMulticastHeartBeatMessage);

class CMulticastHeartBeatMessage::Internel
{
public:
	Internel( ProtoHeader* header, bool isRequest = true )
	{

        m_used_state = -1;
		m_current_state = -1;
		m_master_timestamp = 0;
		m_cluster_id = 0;
		
		m_current_pos = 0;
		m_deserialize_data = NULL;
		m_deserialize_data_len = 0;

		m_header_ptr = header;

		m_header_ptr->type = isRequest ? DS_PROTOCOL_REQ : DS_PROTOCOL_RES;
        m_header_ptr->cmd = HAFK_MULTICAST_HEARTBEAT_MESSAGE;
		m_header_ptr->seq = 1;
		m_header_ptr->bodyLen = 0;
		m_master_log_file_position = 0;
		
		//m_slave_running_state = 0;
		//m_slave_sync_state = 0;
		//m_SQL_mode = 0;
		m_master_init_log_file_position = 0;
		m_master_change_flag = 0;

		m_master_current_binlog_name = "";
		m_master_current_binlog_pos = 0;
		m_master_current_binlog_time = 0;
	}

	~Internel()
	{
		if( NULL != m_deserialize_data ){
			delete[] m_deserialize_data;
			m_deserialize_data = NULL;
			m_deserialize_data_len = 0;
		}
	}

	bool checkPacketHeader( const ProtoHeader* header )
	{
		bool ret =
				( m_header_ptr->type == header->type && m_header_ptr->cmd == header->cmd
						&& m_header_ptr->bodyLen <= header->bodyLen ) ? true : false;

		if( !ret ){
			DBHALOG_ERROR("HeartBeatMessage:%s %d, m_header_ptr->type : %d, header->type : %d,"
					"m_header_ptr->cmd : %d, header->cmd : %d\n", __FUNCTION__, __LINE__,
					m_header_ptr->type, header->type, m_header_ptr->cmd, header->cmd);
		}

		return ret;
	}

	//尾部插入方式增加原始数据(序列化后的)
	bool append( const char* buf, uint32_t len )
	{
		if( NULL == buf || 0 == len )
			return false;
		uint32_t header_len = PROTO_HEAD_SIZE;
		//第一次传递进来的不足一个协议头
		if( 0 == m_current_pos && len < header_len )
			return false;
		//一条完整的协议内容已经填充满,不允许再次使用
		if( m_deserialize_data_len == m_current_pos && 0 != m_current_pos )
			return false;
		if( 0 == m_current_pos ){
			m_sn.setDeserialize( buf, len );
			ProtoHeader tmp_header;
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
		}else{
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
		debugf( "CMulticastHeartBeatMessage: type:%u, cmd:%u seq:%u bodyLength:%u\n", m_header_ptr->type, m_header_ptr->cmd,
				m_header_ptr->seq, m_header_ptr->bodyLen );

		std::ostringstream oss;
		oss << "service name : " << m_service_name
				<< ", used state : " << static_cast<int>( m_used_state )
				<< ", current state : " << static_cast<int>( m_current_state )	<< std::endl
				<< "master time stamp : " << static_cast<int>( m_master_timestamp )
				<< ", uid : " << m_uid
				<< ", cluster id : " << m_cluster_id
				<< ", virtual ip : " << m_vip << std::endl
				<< "local ip : " << m_local_ip
				<< ", peer ip : " << m_peer_ip << std::endl
                << "user defined: " << m_user_defined;

		debugf( "%s", oss.str().c_str() );
		oss.str( "" );
	}
	//重新设置序列号
	void setSequence( uint32_t seq )
	{
		m_header_ptr->seq = seq;
	}

	void setUid( const std::string& uid )
	{
		m_uid = uid;
	}

	std::string getUid() const
	{
		return m_uid;
	}

	void setClusterId(const uint8_t id )
	{
		m_cluster_id = id;
	}

	uint8_t getClusterId() const
	{
		return m_cluster_id;
	}

    void setVirtualIp( const std::string& ip )
	{
		m_vip = ip;
	}

	std::string getVirtualIp() const
	{
		return m_vip;
	}

	void setLocalIp( const std::string& ip )
	{
		m_local_ip = ip;
	}

	std::string getLocalIp() const
	{
		return m_local_ip;
	}

	void setPeerIp( const std::string& ip )
	{
		m_peer_ip = ip;
	}

	std::string getPeerIp() const
	{
		return m_peer_ip;
	}

    void setServiceName( const std::string& serviceName )
	{
		m_service_name = serviceName;
	}

    std::string getServiceName() const
	{
		return m_service_name;
	}

	void setUsedState( const int8_t state )
	{
		m_used_state = state;
	}

	int8_t getUsedState() const
	{
		return m_used_state;
	}

	void setCurrentState( const int8_t state )
	{
		m_current_state = state;
	}

	int8_t getCurrentState() const
	{
		return m_current_state;
	}

	void setMasterTimeStamp( const int32_t timestamp )
	{
		m_master_timestamp = timestamp;
	}

	int32_t getMasterTimeStamp() const
	{
		return m_master_timestamp;
	}

    void setUserDefined( const std::string& ip )
    {
        m_user_defined = ip;
    }

    std::string getUserDefined() const
    {
        return m_user_defined;
    }

	//mysql相关
	void setMasterBinlogName( const std::string& fileName )
	{
		m_master_log_file_name = fileName;
	}

	std::string getMasterBinLogName() const
	{
		return m_master_log_file_name;
	}

	void setMasterBinlogPos( const int32_t pos )
	{
		m_master_log_file_position = pos;
	}
	
	int32_t getMasterBinlogPos() const
	{
		return m_master_log_file_position;
	}

	//void setSlaveSqlState( const int32_t sqlState )
	//{
	//	m_slave_running_state = sqlState;
	//}

	//int32_t getSlaveSqlState() const
	//{
	//	return m_slave_running_state;
	//}

	//void setSlaveSyncState( const int32_t syncState )
	//{
	//	m_slave_sync_state = syncState;
	//}

	//int32_t getSlaveSyncState() const
	//{
	//	return m_slave_sync_state;
	//}

	//void setMysqlMode(const int32_t mode)
	//{
	//	m_SQL_mode = mode;
	//}

	//int32_t getMysqlMode() const
	//{
	//	return m_SQL_mode;
	//}

	void setMasterCurrentBinlogInfo( const std::string binlogName, const int64_t binlogPos, const uint64_t currentTime)
	{
		m_master_current_binlog_name = binlogName;
		m_master_current_binlog_pos = binlogPos;
		m_master_current_binlog_time = currentTime;
	}

	void setMasterInitBinlogName( const std::string& fileName )
	{
		m_master_init_log_file_name = fileName;
	}

	std::string getMasterInitBinlogName() const
	{
		return m_master_init_log_file_name;
	}

	void setMasterInitBinlogPos( const int32_t pos )
	{
		m_master_init_log_file_position = pos;
	}

	int32_t getMasterInitBinlogPos() const
	{
		return m_master_init_log_file_position;
	}
	
	void setMasterChangeFlag(const int32_t flag)
	{
		m_master_change_flag = flag;
	}

	int32_t getMasterChangeFlag() const
	{
		return m_master_change_flag;
	}
	
	std::string getMasterCurrentBinLogName() const
	{
		return m_master_current_binlog_name;
	}

	int64_t getMasterCurrentBinlogPos() const
	{
		return m_master_current_binlog_pos;
	}

	uint64_t getMasterCurrentBinlogTime() const
	{
		return m_master_current_binlog_time;
	}

protected:
    bool serialize()
	{
		m_header_ptr->bodyLen += 3 * INT8_SIZE + 4 * INT32_SIZE +2*INT64_SIZE + sizeOfString(m_uid)
                + sizeOfString(m_vip) + sizeOfString(m_local_ip) + sizeOfString(m_peer_ip) 
                + sizeOfString(m_service_name) + sizeOfString(m_user_defined)+sizeOfString(m_master_log_file_name)
				+ sizeOfString(m_master_init_log_file_name)+sizeOfString(m_master_current_binlog_name);

		int64_t total_len = PROTO_HEAD_SIZE + m_header_ptr->bodyLen;
		m_sn.setSerialize( total_len );

        if( EFS_SUCCESS != m_sn.writeBytes(m_header_ptr, PROTO_HEAD_SIZE) ) 
            return false;

		if( EFS_SUCCESS != m_sn.writeString( m_uid ) )
			return false;

		if( EFS_SUCCESS != m_sn.writeInt8( m_cluster_id ) )
			return false;

        if( EFS_SUCCESS != m_sn.writeString( m_vip ) )
			return false;

        if( EFS_SUCCESS != m_sn.writeString( m_local_ip ) )
			return false;

        if( EFS_SUCCESS != m_sn.writeString( m_peer_ip ) )
			return false;

        if( EFS_SUCCESS != m_sn.writeString( m_service_name ) )
			return false;

        if( EFS_SUCCESS != m_sn.writeInt8( m_used_state ) )
			return false;

        if( EFS_SUCCESS != m_sn.writeInt8( m_current_state ) )
			return false;

        if( EFS_SUCCESS != m_sn.writeInt32( m_master_timestamp ) )
			return false;

        if( EFS_SUCCESS != m_sn.writeString( m_user_defined ) )
            return false;

		if( EFS_SUCCESS != m_sn.writeString( m_master_log_file_name ) )
			return false;
		
		if( EFS_SUCCESS != m_sn.writeInt32( m_master_log_file_position ) )
			return false;

		//if( EFS_SUCCESS != m_sn.writeInt32( m_slave_running_state ) )
		//	return false;

		//if( EFS_SUCCESS != m_sn.writeInt32( m_slave_sync_state ) )
		//	return false;

		if( EFS_SUCCESS != m_sn.writeString( m_master_init_log_file_name ) )
			return false;

		if( EFS_SUCCESS != m_sn.writeInt32( m_master_init_log_file_position ) )
			return false;

		if( EFS_SUCCESS != m_sn.writeInt32( m_master_change_flag ) )
			return false;

		if( EFS_SUCCESS != m_sn.writeInt64( m_master_current_binlog_pos ) )
			return false;

		if( EFS_SUCCESS != m_sn.writeString( m_master_current_binlog_name ) )
			return false;

		if( EFS_SUCCESS != m_sn.writeInt64( m_master_current_binlog_time ) )
			return false;


		return true;
	}

	//对接收的序列化数据,进行反序列化
	bool deserialize()
	{
		m_sn.setDeserialize( m_deserialize_data, m_deserialize_data_len );

        if( EFS_SUCCESS != m_sn.readBytes(m_header_ptr, PROTO_HEAD_SIZE) ) 
            return false;

		if( EFS_SUCCESS != m_sn.readString( m_uid ) )
			return false;

		if( EFS_SUCCESS != m_sn.readInt8( (int8_t*) &m_cluster_id ) )
			return false;

		if( EFS_SUCCESS != m_sn.readString( m_vip ) )
			return false;

		if( EFS_SUCCESS != m_sn.readString( m_local_ip ) )
			return false;

		if( EFS_SUCCESS != m_sn.readString( m_peer_ip ) )
			return false;

		if( EFS_SUCCESS != m_sn.readString( m_service_name ) )
			return false;

		if( EFS_SUCCESS != m_sn.readInt8( (int8_t*) &m_used_state ) )
			return false;

		if( EFS_SUCCESS != m_sn.readInt8( (int8_t*) &m_current_state ) )
			return false;

        if( EFS_SUCCESS != m_sn.readInt32( (int32_t*) &m_master_timestamp ) )
			return false;

        if( EFS_SUCCESS != m_sn.readString( m_user_defined ) )
            return false;

		if( EFS_SUCCESS != m_sn.readString( m_master_log_file_name ) )
			return false;

		if( EFS_SUCCESS != m_sn.readInt32( (int32_t*) &m_master_log_file_position ) )
			return false;

		//if( EFS_SUCCESS != m_sn.readInt32( (int32_t*) &m_slave_running_state ) )
		//	return false;

		//if( EFS_SUCCESS != m_sn.readInt32( (int32_t*) &m_slave_sync_state ) )
		//	return false;

		if( EFS_SUCCESS != m_sn.readString( m_master_init_log_file_name ) )
			return false;

		if( EFS_SUCCESS != m_sn.readInt32( (int32_t*) &m_master_init_log_file_position ) )
			return false;

		if( EFS_SUCCESS != m_sn.readInt32( (int32_t*) &m_master_change_flag ) )
			return false;


		if( EFS_SUCCESS != m_sn.readInt64( (int64_t*) &m_master_current_binlog_pos ) )
			return false;

		if( EFS_SUCCESS != m_sn.readString( m_master_current_binlog_name ) )
			return false;

		if( EFS_SUCCESS != m_sn.readInt64( (int64_t*) &m_master_current_binlog_time ) )
			return false;

		return true;
	}

private:
	//消息内容
	std::string m_uid;              // uid
	uint8_t m_cluster_id;           // cluster id
	std::string m_vip;              // 虚IP
	std::string m_local_ip;         // 本机IP
	std::string m_peer_ip;          // 对端IP
    std::string m_service_name;     // 服务名称
	int8_t m_used_state;            // 本机历史状态
	int8_t m_current_state;         // 本机当前状态
	int32_t m_master_timestamp;     // 本机成为master的时间戳
    std::string m_user_defined;     //用户自定义字段
	//mysql相关配置
	std::string m_master_log_file_name; //主机binlog文件名
	int32_t m_master_log_file_position; //主机binlog位置

	//int32_t m_slave_running_state;  //备机的sql线程状态
	//int32_t m_slave_sync_state;    //备机同步状态

	//int32_t m_SQL_mode;			//mysql服务主备状态

	std::string m_master_init_log_file_name; //主机初始化的binlog文件名
	int32_t m_master_init_log_file_position; //主机初始化的binlog位置

	int32_t m_master_change_flag;			//主机是否合法成为主的标记

	int64_t m_master_current_binlog_pos;					// 当前binlog的位置
	std::string m_master_current_binlog_name;				// 当前binlog的名字
	uint64_t m_master_current_binlog_time;					// 获取当前binlog信息的时间


private:
	//类使用的变量
	ProtoHeader* m_header_ptr;
	//存储序列化数据和提供序列化和反序列化接口
	Serialization m_sn;
	uint32_t m_current_pos; //记录反序列数据插入的位置
	char* m_deserialize_data;
	uint32_t m_deserialize_data_len;
};

////////////////////////////////////////////////////////////////////////////////
CMulticastHeartBeatMessage::CMulticastHeartBeatMessage( bool request, uint32_t seq )
{
	m_internel = new Internel( &m_header, request );
	assert( NULL != m_internel );
	m_internel->setSequence( seq );
}

CMulticastHeartBeatMessage::~CMulticastHeartBeatMessage()
{
	if( NULL != m_internel )
		delete m_internel;
	m_internel = NULL;
}

bool CMulticastHeartBeatMessage::append( const char* buf, uint32_t len )
{
	return m_internel->append( buf, len );
}

const char* CMulticastHeartBeatMessage::getData()
{
	return m_internel->getData();
}

uint32_t CMulticastHeartBeatMessage::getDataLen()
{
	return m_internel->getDataLen();
}

void CMulticastHeartBeatMessage::printPacket() const
{
	m_internel->printPacket();
}

void CMulticastHeartBeatMessage::setUid( const std::string& uid )
{
	return m_internel->setUid( uid );
}

std::string CMulticastHeartBeatMessage::getUid() const
{
	return m_internel->getUid();
}

void CMulticastHeartBeatMessage::setClusterId( const uint8_t id )
{
	return m_internel->setClusterId( id );
}

uint8_t CMulticastHeartBeatMessage::getClusterId() const
{
	return m_internel->getClusterId();
}

void CMulticastHeartBeatMessage::setVirtualIp( const std::string& ip )
{
	return m_internel->setVirtualIp( ip );
}

std::string CMulticastHeartBeatMessage::getVirtualIp() const
{
	return m_internel->getVirtualIp();
}

void CMulticastHeartBeatMessage::setLocalIp( const std::string& ip )
{
	return m_internel->setLocalIp( ip );
}

std::string CMulticastHeartBeatMessage::getLocalIp() const
{
	return m_internel->getLocalIp();
}

void CMulticastHeartBeatMessage::setPeerIp( const std::string& ip )
{
	return m_internel->setPeerIp( ip );
}

std::string CMulticastHeartBeatMessage::getPeerIp() const
{
	return m_internel->getPeerIp();
}

void CMulticastHeartBeatMessage::setServiceName( const std::string& serviceName )
{
	return m_internel->setServiceName( serviceName );
}

std::string CMulticastHeartBeatMessage::getServiceName() const
{
	return m_internel->getServiceName();
}

void CMulticastHeartBeatMessage::setUsedState( const int8_t state )
{
	return m_internel->setUsedState( state );
}

int8_t CMulticastHeartBeatMessage::getUsedState() const
{
	return m_internel->getUsedState();
}

void CMulticastHeartBeatMessage::setCurrentState( const int8_t state )
{
	return m_internel->setCurrentState( state );
}

int8_t CMulticastHeartBeatMessage::getCurrentState() const
{
	return m_internel->getCurrentState();
}

void CMulticastHeartBeatMessage::setMasterTimeStamp( const int32_t timeStamp )
{
	return m_internel->setMasterTimeStamp( timeStamp );
}

int32_t CMulticastHeartBeatMessage::getMasterTimeStamp() const
{
	return m_internel->getMasterTimeStamp();
}

void CMulticastHeartBeatMessage::setUserDefined( const std::string& userDefined )
{
    return m_internel->setUserDefined( userDefined );
}

std::string CMulticastHeartBeatMessage::getUserDefined() const
{
    return m_internel->getUserDefined();
}

//mysql相关
void CMulticastHeartBeatMessage::setMasterBinlogName( const std::string& fileName )
{
	return m_internel->setMasterBinlogName(fileName);
}

std::string CMulticastHeartBeatMessage::getMasterBinLogName() const
{
	return m_internel->getMasterBinLogName();
}

void CMulticastHeartBeatMessage::setMasterBinlogPos( const int32_t pos )
{
	return m_internel->setMasterBinlogPos(pos);
}

int32_t CMulticastHeartBeatMessage::getMasterBinlogPos() const
{
	return m_internel->getMasterBinlogPos();
}

void CMulticastHeartBeatMessage::setMasterCurrentBinlog( const std::string& fileName, const int64_t pos, const uint64_t currentTime )
{
	return m_internel->setMasterCurrentBinlogInfo(fileName, pos, currentTime);
}

std::string CMulticastHeartBeatMessage::getMasterCurrentBinLogName() const
{
	return m_internel->getMasterCurrentBinLogName();
}

int32_t CMulticastHeartBeatMessage::getMasterCurrentBinlogPos() const
{
	return m_internel->getMasterCurrentBinlogPos();
}

uint64_t CMulticastHeartBeatMessage::getMasterCurrentBinlogTime() const
{
	return m_internel->getMasterCurrentBinlogTime();
}

//void CMulticastHeartBeatMessage::setSlaveSqlState( const int32_t sqlState )
//{
//	return m_internel->setSlaveSqlState(sqlState);
//}

//int32_t CMulticastHeartBeatMessage::getSlaveSqlState() const
//{
//	return m_internel->getSlaveSqlState();
//}

//void CMulticastHeartBeatMessage::setSlaveSyncState( const int32_t syncState )
//{
//	return m_internel->setSlaveSyncState(syncState);
//}
//
//int32_t CMulticastHeartBeatMessage::getSlaveSyncState() const
//{
//	return m_internel->getSlaveSyncState();
//}

//void CMulticastHeartBeatMessage::setMysqlMode(const int32_t mysqlMode)
//{
//	return m_internel->setMysqlMode(mysqlMode);
//}
//
//int32_t CMulticastHeartBeatMessage::getMysqlMode() const
//{
//	return m_internel->getMysqlMode();
//}

void CMulticastHeartBeatMessage::setMasterInitBinlogName( const std::string& fileName )
{
	return m_internel->setMasterInitBinlogName(fileName);
}

std::string CMulticastHeartBeatMessage::getMasterInitBinlogName() const
{
	return m_internel->getMasterInitBinlogName();
}

void CMulticastHeartBeatMessage::setMasterInitBinlogPos( const int32_t pos )
{
	return m_internel->setMasterInitBinlogPos(pos);
}

int32_t CMulticastHeartBeatMessage::getMasterInitBinlogPos() const
{
	return m_internel->getMasterInitBinlogPos();
}

void CMulticastHeartBeatMessage::setMasterChangeFlag( const int32_t flag )
{
	return m_internel->setMasterChangeFlag(flag);
}

int32_t CMulticastHeartBeatMessage::getMasterChangeFlag() const
{
	return m_internel->getMasterChangeFlag();
}

///////////////////////////////////////////////////////////////////////////////
TPacketBasePtr newMulticastHeartBeatMessageRequest()
{
    return TPacketBasePtr(new CMulticastHeartBeatMessage(true));
}

TPacketBasePtr newMulticastHeartBeatMessageResponse()
{
    return TPacketBasePtr(new CMulticastHeartBeatMessage(false));
}


} // DBHA
} // VideoCloud
} // Dahua

