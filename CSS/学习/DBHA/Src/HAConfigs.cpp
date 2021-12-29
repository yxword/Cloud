#include "HAConfigs.h"
#include "Json/json.h"
#include "Infra/Guard.h"
#include "Infra/File.h"
#include "HALog.h"
#include "CommonFun.h"

#define CONFIG_FILE "HAConfig.conf"
#define STATE_FILE  "HAState.stat"
#define SQL_FILE    "Sql.stat"

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

DBHALOG_CLASSNAME( CHAConfigs );

using namespace std;
using namespace Infra;

#define  SET_JSON_VAL( root, key)  root[#key] = m_##key

CHAConfigs::CHAConfigs( void )
{
	m_cluster_id = 0;
	m_master_wait_salve_time = 600;
	m_multicast_time_interval_ms = 500;
	m_master_down_interval_ms = 2500;

	m_sync_heartbeat_interval_ms = 3000;
	m_sync_heartbeat_timeout_num = 3;
	m_sync_connect_timeout = 3000;
	m_sync_call_timeout = 3000;
	m_sync_version = 1.0;

	m_used_state = 0;
	m_master_timestamp = 0;

	m_new_machine_flag = false;
	m_SQL_write_state = 0;
	m_SQL_init_binlog_position = 0;
	m_master_change_flag = -1;
	m_master_change_binlog_postion = 0;
	m_slave_sync_info = SyncState[0];
	m_slave_connect_master = SlaveConnectState[0];
	m_SQL_mode = -1;
}

CHAConfigs::~CHAConfigs( void )
{
}

CHAConfigs& CHAConfigs::instance()
{
	static CHAConfigs s_config;
	return s_config;
}

//设置工作目录
void CHAConfigs::setWorkspace( const std::string& path )
{
    this->m_workspace = path;
}

void CHAConfigs::setHAConfig( HAConfig& config )
{
    this->m_HAConfig = config;
}

bool CHAConfigs::writeToFile( const std::string& fileName, const std::string& content )
{
    CFile file;
    if( !file.open( fileName.c_str(), CFile::modeWrite ) )
    {
        DBHALOG_ERROR( "Failed to open config file( %s ).", fileName.c_str() );
        return false;
    }

    size_t total_data_len = content.length();
    size_t data_written = 0;

    while ( data_written < total_data_len ) 
    {
        char * ptr = const_cast<char *>( content.c_str() );
        int ret = file.write( ptr + data_written, total_data_len - data_written );
        if ( ret >= 0 ) // 0 表示没有写入数据，可能是系统忙或已写
        {  
            data_written += ret;
            continue;
        }
        else 
        {
            // 写失
            DBHALOG_ERROR( "Failed to write config file(%s).\n", fileName.c_str() );
            file.close();
            return false;
        }
    }
    file.flush();
    file.close();

    return true;
}

bool CHAConfigs::readFromFile( const std::string& fileName, std::string& content )
{
    CFile file;
    //打开配置文件
    if( !file.open( fileName.c_str(), CFile::modeRead | CFile::typeText ) )
    {
        DBHALOG_ERROR( "Failed to open config file(%s).",  fileName.c_str() );
        return false;
    }

    //读文件内
    char buf[2048] = {0};
    int ret = 0;
    while( ( ret = file.read( buf, 2048 ) ) > 0 )
    {
        content.append(buf, ret);
    }

    file.close();
    return true;
}

bool CHAConfigs::saveToJsonFile( const Json::Value& root, const std::string& fileName, bool safety )
{
    DBHALOG_INFO( "save to json file: %s, safety: %d", fileName.c_str(), safety );

    if ( root.empty() )
    {
        DBHALOG_ERROR("Empty Json Value.");
        return false;
    }

    string js_output;
    Json::StyledWriter js_writer( js_output );
    if ( !js_writer.write( root ) ) {
        DBHALOG_ERROR( "Json StyledWriter failed to write config info." );
        return false;
    }

    //安全模式, 先写.bak
    if( safety )
    {
        string bak_file_name = fileName + ".bak";
        string prev_file_name = fileName + ".prev";

        if( ! writeToFile( bak_file_name, js_output ) )
            return false;

        // 将原有文件改名为 file.prev  
        // 将file.bak 文件改为 file. 
        // 如rename失败，暂定只打日志，不返回失败

        CFile::remove( prev_file_name.c_str() );        //windows上rename无法覆盖已存在的文件
        if ( !CFile::rename( fileName.c_str(), prev_file_name.c_str() ) )
        {
            DBHALOG_ERROR( "can't rename file %s to %s\n", fileName.c_str(), prev_file_name.c_str() );
        }

        CFile::remove( fileName.c_str() );           //windows上rename无法覆盖已存在的文件
        if ( !CFile::rename( bak_file_name.c_str(), fileName.c_str() ) )
        {
            DBHALOG_ERROR( "can't rename file %s to %s\n", bak_file_name.c_str(), fileName.c_str() );
        }

        return true;
    }
    else
    {
        return writeToFile( fileName, js_output );
    }
}


bool CHAConfigs::loadFromJsonFile( const std::string& fileName, Json::Value& value )
{

    string prev_file_name = fileName + ".prev";
    string content;

    Json::Reader reader;
    bool ret = readFromFile( fileName, content ) && reader.parse( content, value );

    //加载失败, 尝试从prev文件加载
    if ( !ret )
    {
        DBHALOG_ERROR( "load from config file( %s ) failed, try load from prev file. Error Msg: %s.", fileName.c_str(),
            reader.getFormatedErrorMessages().c_str() );

        ret = readFromFile( prev_file_name.c_str(), content ) && reader.parse( content, value );

        //从prev文件中加载 将prev文件重命名为
        if( ret )
        {
            DBHALOG_DEBUG( "load from prev config file( %s ) success, try rename prev file.", prev_file_name.c_str() );

            CFile::remove( fileName.c_str() );           //windows上rename无法覆盖已存在的文件
            if ( !CFile::rename( prev_file_name.c_str(), fileName.c_str() ) )
            {
                DBHALOG_ERROR( "can't rename file %s to %s\n", prev_file_name.c_str(), fileName.c_str() );
            }
        }
        else
        {
            DBHALOG_ERROR( "load from config file( %s ) failed, try load from prev file. Error Msg: %s.", prev_file_name.c_str(),
                reader.getFormatedErrorMessages().c_str() );
        }
    }

    return ret;
}


#define  GET_JSON_VAL( root, val, jsontype, defaultVal )    do { \
    if ( !root.isMember( #val ) )  {	\
		DBHALOG_DEBUG( "key ( %s ) does not exist, set as default value.", #val ); \
        m_##val = defaultVal; \
        ret = false; \
    } \
    else { \
        m_##val = root[#val].jsontype(); \
    } \
} while ( 0 )

//加载数据库能否成为主的状态
bool CHAConfigs::loadSqlState()
{
	// todo 状态配置文件路径做成可配置
	CGuard guard( m_sqlFileMutex );
	string file_name = m_workspace;
	file_name.append( "/" );
	file_name.append( SQL_FILE );

	bool ret = loadFromJsonFile( file_name, m_sqlRoot );

	GET_JSON_VAL( m_sqlRoot, SQL_write_state, asInt, 0 );

	GET_JSON_VAL( m_sqlRoot, SQL_init_binlog_file, asString, "" );
	GET_JSON_VAL( m_sqlRoot, SQL_init_binlog_position, asInt, 0 );

	GET_JSON_VAL( m_sqlRoot, master_change_flag, asInt, -1 );
	GET_JSON_VAL( m_sqlRoot, master_change_binlog_file, asString, "" );
	GET_JSON_VAL( m_sqlRoot, master_change_binlog_postion, asInt, 0 );

	GET_JSON_VAL( m_sqlRoot, slave_sync_info, asString, SyncState[0] );
	GET_JSON_VAL( m_sqlRoot, slave_connect_master, asString, SlaveConnectState[2] );

	GET_JSON_VAL( m_sqlRoot, SQL_mode, asInt, -1 );

	return ret;
}

//保存sql配置，todo是否
bool CHAConfigs::saveSqlState()
{
	CGuard guard( m_sqlFileMutex );

	string file_name = m_workspace;
	file_name.append( "/" );
	file_name.append( SQL_FILE );

	//构造json
	SET_JSON_VAL( m_sqlRoot, SQL_write_state );
	SET_JSON_VAL( m_sqlRoot, SQL_init_binlog_file );
	SET_JSON_VAL( m_sqlRoot, SQL_init_binlog_position );
	SET_JSON_VAL( m_sqlRoot, master_change_flag );
	SET_JSON_VAL( m_sqlRoot, master_change_binlog_file );
	SET_JSON_VAL( m_sqlRoot, master_change_binlog_postion );
	SET_JSON_VAL( m_sqlRoot, slave_sync_info );
	SET_JSON_VAL( m_sqlRoot, slave_connect_master );
	SET_JSON_VAL( m_sqlRoot, SQL_mode );

	return saveToJsonFile( m_sqlRoot, file_name.c_str(), true );
}

bool CHAConfigs::setSQLWriteState(int state)
{
	CGuard guard( m_sqlFileMutex );

	string file_name = m_workspace;
	file_name.append( "/" );
	file_name.append( SQL_FILE );

	m_SQL_write_state = state;

	SET_JSON_VAL( m_sqlRoot, SQL_write_state );

	return saveToJsonFile( m_sqlRoot, file_name.c_str(), true );
}

bool CHAConfigs::setSQLMode(int mode)
{
	CGuard guard( m_sqlFileMutex );
	string file_name = m_workspace;
	file_name.append( "/" );
	file_name.append( SQL_FILE );

	m_SQL_mode = mode;
	SET_JSON_VAL( m_sqlRoot, SQL_mode );

	if (m_SQL_mode == NotSlaveNotMaster)
		m_SQL_write_state = NotReadNotWrite;
	else if (m_SQL_mode == Master)
		m_SQL_write_state = WriteState;
	else if (m_SQL_mode == Slave)
		m_SQL_write_state = ReadState;

	SET_JSON_VAL(m_sqlRoot, SQL_write_state);

	return saveToJsonFile( m_sqlRoot, file_name.c_str(), true );
}

int32_t CHAConfigs::getSQLMode()
{
	loadSqlState();
	return m_SQL_mode;
}

// 单机模式下主备等待为1s，双机等待时间为600s
//void CHAConfigs::setSingleMode(bool flag)
//{
//	/*if (flag==false)
//		m_master_wait_salve_time = 600;
//	else
//		m_master_wait_salve_time = 1;*/
//	DBHALOG_WARN("m_master_wait_salve_time is %d ", m_master_wait_salve_time);
//}

bool CHAConfigs::newMachine()
{
	return m_new_machine_flag;
}

void CHAConfigs::getInitBinlogInfo(std::string& fileName, int32_t& postion)
{
	//loadSqlState();
	fileName = m_SQL_init_binlog_file;
	postion = m_SQL_init_binlog_position;
}

bool CHAConfigs::setInitBinlogInfo(std::string binlogInfo)
{
	
	CGuard guard( m_sqlFileMutex );

	string file_name = m_workspace;
	file_name.append( "/" );
	file_name.append( SQL_FILE );

	if (!CommonFun::setBinlogInfo(binlogInfo, m_SQL_init_binlog_file, m_SQL_init_binlog_position))
		return false;

	SET_JSON_VAL( m_sqlRoot, SQL_init_binlog_file );
	SET_JSON_VAL( m_sqlRoot, SQL_init_binlog_position );

	return saveToJsonFile( m_sqlRoot, file_name.c_str(), true );

}
//
//bool CHAConfigs::setChangeMasterInfo(std::string binlogInfo,int32_t flag)
//{
//	/*binglog返回信息是filename pos*/
//	if ( binlogInfo == "" )
//	{
//		DBHALOG_ERROR("binloginfo is empty");
//		return false;
//	}
//	string bin_log_info = binlogInfo;
//	StringVec itemvec;
//	CommonFun::strSplit( bin_log_info, ' ', itemvec );
//	string binlog_filename = itemvec[0];
//	string pos = itemvec[1];
//	int32_t bin_pos = atoi(pos.c_str());
//	DBHALOG_INFO("change master bin log file %s, file pos %d,flag :%d",binlog_filename.c_str(),bin_pos,flag);
//	CGuard guard( m_sqlFileMutex );
//
//	string file_name = m_workspace;
//	file_name.append( "/" );
//	file_name.append( SQL_FILE );
//
//	m_master_change_binlog_file = binlog_filename;
//	m_master_change_binlog_postion = bin_pos;
//	m_master_change_flag = flag;
//
//	SET_JSON_VAL( m_sqlRoot, master_change_flag );
//	SET_JSON_VAL( m_sqlRoot, master_change_binlog_file );
//	SET_JSON_VAL( m_sqlRoot, master_change_binlog_postion );
//
//	return saveToJsonFile( m_sqlRoot, file_name.c_str(), true );
//
//}

void CHAConfigs::setChangeMasterInfo(const std::string& localBinlogInfo, int32_t localPosition)
{
	DBHALOG_INFO("file:%s:%d", localBinlogInfo.c_str(), localPosition);
	m_master_change_binlog_file = localBinlogInfo;
	m_master_change_binlog_postion = localPosition;

	CGuard guard( m_sqlFileMutex );
	string file_name = m_workspace;
	file_name.append( "/" );
	file_name.append( SQL_FILE );
	SET_JSON_VAL( m_sqlRoot, master_change_binlog_file );
	SET_JSON_VAL( m_sqlRoot, master_change_binlog_postion );
	saveToJsonFile( m_sqlRoot, file_name.c_str(), true );
}

void CHAConfigs::getChangeMasterInfo(int32_t& flag, std::string& fileName, int32_t& postion)
{
	//loadSqlState();
	flag = m_master_change_flag;
	fileName = m_master_change_binlog_file;
	postion = m_master_change_binlog_postion;
}

bool CHAConfigs::getSlaveConnectMasterState()
{
	//loadSqlState();
	if (getSlaveConnectState() != SlaveConnectState[1])
		return false;
	else
		return true;
}

// true表示已经同步完成，false标志未同步
bool CHAConfigs::getSlaveChangeMasterFlag()
{
	//loadSqlState();

	std::string slave_sync_info = getSlaveSyncState();
	if ((slave_sync_info != SyncState[2])){
		DBHALOG_INFO("slave change flag:%s", slave_sync_info.c_str());
		return false;
	}else{
		return true;
	}
}

bool CHAConfigs::setSlaveConnectState(std::string connectState)
{
	CGuard guard( m_sqlFileMutex );

	string file_name = m_workspace;
	file_name.append( "/" );
	file_name.append( SQL_FILE );

	m_slave_connect_master = connectState;

	SET_JSON_VAL( m_sqlRoot, slave_connect_master );

	//DBHALOG_INFO("connect state:%s", connectState.c_str());

	return saveToJsonFile( m_sqlRoot, file_name.c_str(), true );
}

bool CHAConfigs::setSlaveSyncState(std::string syncInfo)
{	
	CGuard guard( m_sqlFileMutex );

	// 如果相同则不需要进行文件的写入
	if (syncInfo == m_slave_sync_info)
		return true;

	string file_name = m_workspace;
	file_name.append( "/" );
	file_name.append( SQL_FILE );

	m_slave_sync_info = syncInfo;

	SET_JSON_VAL( m_sqlRoot, slave_sync_info );

	//DBHALOG_INFO("syncInfo:%s", syncInfo.c_str());

	return saveToJsonFile( m_sqlRoot, file_name.c_str(), true );
}

std::string CHAConfigs::getSlaveConnectState()
{
	CGuard guard( m_sqlFileMutex );

	return m_slave_connect_master;
}

std::string CHAConfigs::getSlaveSyncState()
{
	CGuard guard( m_sqlFileMutex );

	return m_slave_sync_info;
}


//加载配置文件.
bool CHAConfigs::loadConfig()
{
    string file_name = m_workspace;
    file_name.append( "/" );
    file_name.append( CONFIG_FILE );

    bool ret = loadFromJsonFile( file_name, m_configRoot );

    GET_JSON_VAL( m_configRoot, uid, asString, "" );
	if (m_uid == "")
		m_new_machine_flag = true;
	else
		m_new_machine_flag = false;
    GET_JSON_VAL( m_configRoot, cluster_id, asUInt, 0 );
    GET_JSON_VAL( m_configRoot, master_wait_salve_time, asUInt, 600 );
    GET_JSON_VAL( m_configRoot, multicast_time_interval_ms, asUInt, 500 );
    GET_JSON_VAL( m_configRoot, master_down_interval_ms, asUInt, 2500 );

    GET_JSON_VAL( m_configRoot, sync_heartbeat_interval_ms, asUInt, 3000 );
    GET_JSON_VAL( m_configRoot, sync_heartbeat_timeout_num, asUInt, 3 );
    GET_JSON_VAL( m_configRoot, sync_connect_timeout, asUInt, 3000 );
    GET_JSON_VAL( m_configRoot, sync_call_timeout, asUInt, 3000 );
    GET_JSON_VAL( m_configRoot, sync_version, asDouble, 1.0 );

    return ret;
}


//加载HA状态
bool CHAConfigs::loadHAState()
{
    string file_name = m_workspace;
    file_name.append( "/" );
    file_name.append( STATE_FILE );

    bool ret = loadFromJsonFile( file_name, m_stateRoot );

    GET_JSON_VAL( m_stateRoot, used_state, asInt, 0 );
    GET_JSON_VAL( m_stateRoot, master_timestamp, asInt, 0 );

    return ret;
}


//保存配置文件
bool CHAConfigs::saveConfig()
{
    CGuard guard( m_configFileMutex );

    string file_name = m_workspace;
    file_name.append( "/" );
    file_name.append( CONFIG_FILE );

    //构造json
    SET_JSON_VAL( m_configRoot, uid );
    SET_JSON_VAL( m_configRoot, cluster_id );
    SET_JSON_VAL( m_configRoot, master_wait_salve_time );
    SET_JSON_VAL( m_configRoot, multicast_time_interval_ms );
    SET_JSON_VAL( m_configRoot, master_down_interval_ms );

    SET_JSON_VAL( m_configRoot, sync_heartbeat_interval_ms );
    SET_JSON_VAL( m_configRoot, sync_heartbeat_timeout_num );
    SET_JSON_VAL( m_configRoot, sync_connect_timeout );
    SET_JSON_VAL( m_configRoot, sync_call_timeout );
    m_configRoot["sync_version"] = ( double )m_sync_version;

    return saveToJsonFile( m_configRoot, file_name.c_str(), true );
}

//保存HA状态
bool CHAConfigs::saveHAState( int32_t usedState, int32_t masterTimestamp )
{
    m_used_state = usedState;
    m_master_timestamp = masterTimestamp;

    CGuard guard( m_stateFileMutex );

    string file_name = m_workspace;
    file_name.append( "/" );
    file_name.append( STATE_FILE );

    //构造json
    SET_JSON_VAL( m_stateRoot, used_state );
    SET_JSON_VAL( m_stateRoot, master_timestamp );

    return saveToJsonFile( m_stateRoot, file_name.c_str(), false );
}


//////////////////////////////////////////////////////////////////////////

const std::string CHAConfigs::getUId(){ return m_uid; }
void CHAConfigs::setUId( const std::string uid ){ this->m_uid = uid;}

uint8_t CHAConfigs::getClusterId(){ return m_cluster_id; }
void CHAConfigs::setClusterId( uint8_t clusterId ){ this->m_cluster_id = clusterId;}

uint32_t CHAConfigs::getMulticastTimeInterval(){ return m_multicast_time_interval_ms; }
uint32_t CHAConfigs::getMasterDownInterval(){ return m_master_down_interval_ms; }

int32_t CHAConfigs::getUsedState(){ return m_used_state; }
int32_t CHAConfigs::getMasterTimestamp(){ return m_master_timestamp; }


uint32_t CHAConfigs::getSyncHeartbeatInterval() { return m_sync_heartbeat_interval_ms; }
uint32_t CHAConfigs::getSyncHeartbeatTimeout() { return m_sync_heartbeat_timeout_num; }
uint32_t CHAConfigs::getSyncConnectTimeout() { return m_sync_connect_timeout; }
uint32_t CHAConfigs::getSyncCallTimeout() { return m_sync_call_timeout; }
uint32_t CHAConfigs::getMasterWaitSlaveTime(){return m_master_wait_salve_time;} 

uint64_t CHAConfigs::getSyncVersion() { return m_sync_version; }
void CHAConfigs::increaseSyncVersion(){ m_sync_version++;}
void CHAConfigs::setSyncVersion( uint64_t version ){ this->m_sync_version = version; }


//HAConfig
uint32_t CHAConfigs::getSyncServerPort() { return m_HAConfig.syncPort; }
std::string CHAConfigs::getMulticastIP(){ return m_HAConfig.multicastIP; }          
uint32_t CHAConfigs::getMulticastPort(){ return m_HAConfig.multicastPort; }         
std::string CHAConfigs::getLocalHeartbeatIP(){ return m_HAConfig.localHeartbeatIP; }
std::string CHAConfigs::getPeerHeartbeatIP(){ return m_HAConfig.peerHeartbeatIP; }  
std::string CHAConfigs::getLocalBusinessIP(){ return m_HAConfig.localBusinessIP; }  
std::string CHAConfigs::getPeerBusinessIP(){ return m_HAConfig.peerBusinessIP; }    
std::string CHAConfigs::getVIP(){ return m_HAConfig.vip; }                         
std::string CHAConfigs::getVIPNetmask(){ return m_HAConfig.vipNetmask; }            
std::string CHAConfigs::getVIFName(){ return m_HAConfig.vifName; }                  
std::string CHAConfigs::getIFName(){ return m_HAConfig.ifName; }  
std::string CHAConfigs::getServiceName(){ return m_HAConfig.serviceName; }          
std::string CHAConfigs::getUserDefined(){ return m_HAConfig.userDefined; }          

//////////////////////////////////////////////////////////////////////////

} // DBHA
} // VideoCloud
} // Dahua

