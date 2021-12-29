#include "SyncClient.h"
#include "HALog.h"
#include "HAConfigs.h"
#include "NetFramework/SockAddrStorage.h"
#include "Common/ProtoParser.h"
#include "MessageType.h"
#include "HAHeartbeatMessage.h"
#include "SyncDumpFileInfoMessage.h"
#include "SyncBeginMessage.h"
#include "SyncDataMessage.h"
#include "SyncEndMessage.h"
#include "Common/Defs.h"
#include "Common/PacketBase.h"
#include "HAErrorCode.h"
#include "CommonFun.h"

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

DBHALOG_CLASSNAME( CSyncClient );

#define DELETE_STREAM( a ) if( a ){delete a; a=NULL;}

const std::string recovery_file_suffix=".recovery";

using namespace std;

CSyncClient::CSyncClient( ) 
:m_timer( "CSyncClient timer" )
{
	m_is_started = false;
	m_exit = true;
	m_stream = NULL;
	m_close_sem = NULL;
	m_heartbeat_timeout = 0;
	m_sync_state = offline;
	m_pre_recovery_file_size = 0;
	m_current_recover_binlog_file = "";
	m_current_recover_binlog_pos = 0;
	m_wait_dump_threads = 0;

	pthread_mutex_init(&m_dumping_mutex, NULL);
	pthread_cond_init(&m_dumping_cond, NULL);
}

CSyncClient::~CSyncClient( void )
{
	cleanup();
	pthread_cond_destroy( &m_dumping_cond );
	pthread_mutex_destroy( &m_dumping_mutex );
}

bool CSyncClient::startup( )
{
	if( m_is_started ){
		DBHALOG_ERROR( "sync client is started." );
		return false;
	}

	//注册相关协议.重复注册会用最新的值.
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_REQ, HAFK_SYNC_BEGIN_MESSAGE, newCSyncBeginMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_RES, HAFK_SYNC_BEGIN_MESSAGE, newCResSyncBeginMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_REQ, HAFK_SYNC_DATA_MESSAGE, newCSyncDataMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_RES, HAFK_SYNC_DATA_MESSAGE, newCResSyncDataMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_REQ, HAFK_SYNC_END_MESSAGE, newCSyncEndMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_RES, HAFK_SYNC_END_MESSAGE, newCResSyncEndMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_REQ, HAFK_HEARTBEAT_MESSAGE, newCHAHeartbeatMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_RES, HAFK_HEARTBEAT_MESSAGE, newCResHAHeartbeatMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_RES, HAFK_HEARTBEAT_MESSAGE, newCResHAHeartbeatMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_REQ, HAFK_SYNC_DUMP_FILE_INFO_MESSAGE, newCSyncDumpFileInfoMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_RES, HAFK_SYNC_DUMP_FILE_INFO_MESSAGE, newCResSyncDumpFileInfoMessage );
	//心跳定时器.
	Infra::CTimer::Proc proc( &CSyncClient::onStreamTimer, this );
	if( !m_timer.start( proc, 0, gHAConfig.getSyncHeartbeatInterval() ) ){
		DBHALOG_ERROR( "start timer failed." );
		return false;
	}

	m_exit = false;
	m_is_started = true;
	DBHALOG_INFO("init success.");
	return true;
}

void CSyncClient::waitForDumping()
{
	pthread_mutex_lock( &m_dumping_mutex );
	++m_wait_dump_threads;
	pthread_cond_wait( &m_dumping_cond, &m_dumping_mutex);
	pthread_mutex_unlock( &m_dumping_mutex );
}

void CSyncClient::wakeupDumping()
{
	DBHALOG_INFO( "wake up waiting dump thread %d!", m_wait_dump_threads);
	pthread_mutex_lock( &m_dumping_mutex );
	if( m_wait_dump_threads > 0 ){
		pthread_cond_broadcast( &m_dumping_cond );
		m_wait_dump_threads = 0;
		DBHALOG_INFO( "wake up all waiting dump thread!");
	}
	pthread_mutex_unlock( &m_dumping_mutex );
}

bool CSyncClient::cleanup()
{
	m_exit = true;

	if( !m_is_started ){
		DBHALOG_ERROR( "sync client is not started." );
		return false;
	}

	wakeupDumping();

	m_timer.stopAndWait();

	disconnect();

	m_is_started = false;
	
	DBHALOG_INFO( "cleanup synclient success." );
	return true;
}

void CSyncClient::onStreamTimer( unsigned long )
 {	
	//同步发生异常,主动断线再重连.
	if( m_sync_state==syncError ){
		DBHALOG_ERROR( "sync occur error, reconnect." );
		disconnect();
		return;
	}

	//心跳超时断线.
	if( m_heartbeat_timeout>gHAConfig.getSyncHeartbeatTimeout() ){
		DBHALOG_ERROR( "heartbeat[%d > %d] timeout.", m_heartbeat_timeout, gHAConfig.getSyncHeartbeatTimeout() );
		// 所有在恢复中途有问题的，都靠心跳超时来进行恢复任务唤醒
		wakeupDumping();
		m_sync_state = syncError;
		disconnect();
		return;
	}

	//加锁,防止resetlink时在new stream.以及close时候在send.
	Infra::CGuard guard( m_close_mutex );

	m_server_ip = gHAConfig.getPeerHeartbeatIP();
	//优先心跳.
	if( connect() ){
		heartbeat();
		return;
	}

	m_server_ip = gHAConfig.getPeerBusinessIP();
	// 换个线重新连接
	if( connect() ){
		heartbeat();
	}
}

bool CSyncClient::connect()
{
	//没有断线不需要连接.
	if( m_sync_state!=offline ){
		return true;
	}

	//创建流.
	m_stream = new EFS::CStream;
	if( !m_stream ){
		DBHALOG_ERROR( "alloc EFS::CStream failed." );
		return false;
	}
	
	//同步连接.
	if( !m_stream->setConnectOption( gHAConfig.getSyncConnectTimeout(), true ) ){
		DBHALOG_ERROR( "set connect option failed." );
		DELETE_STREAM( m_stream );
		return false;
	}

	//开始连接.
	NetFramework::CSockAddrStorage serverAddr( m_server_ip.c_str(), gHAConfig.getSyncServerPort() );
	EFS::CStream::StateProc proc( &CSyncClient::onStreamState ,this );
	if( m_stream->connect( proc, &serverAddr, NULL )<0 ){
		DBHALOG_ERROR( "[%s:%d] connect failed", m_server_ip.c_str(), gHAConfig.getSyncServerPort() );
		DELETE_STREAM( m_stream );
		return false;
	}

	//重置心跳计数与状态.
	m_heartbeat_timeout = 0;
	m_sync_state = unsynced;
	DBHALOG_INFO( "connect success, state change to unsynced." );

	return true;
}

bool CSyncClient::disconnect()
{
	DBHALOG_INFO( "enter to disconnect." );
	//信号量操作加锁保护,保证在close时候,stream没有被delete.
	m_close_mutex.enter();
	if( !m_stream ){
		m_close_mutex.leave();
		return false;
	}

	m_close_sem = new Infra::CSemaphore();
	ASSERT_ABORT( m_close_sem!=NULL );

	m_stream->close();
	m_close_mutex.leave();

	m_close_sem->pend();

	m_close_mutex.enter();
	delete m_close_sem;
	m_close_sem = NULL;
	m_close_mutex.leave();

	return true;
}

bool CSyncClient::heartbeat()
{
	THAHeartbeatMsgPtr pkt = dynamic_pointer_cast<CHAHeartbeatMessage>( newCHAHeartbeatMessage() );
	if( pkt==NULL ){
		DBHALOG_ERROR( "newCHAHeartbeatMessage failed." );
		return false;
	}

	//外面有加锁,并且保证没有offline.
	if( m_stream->send( pkt )<0 ){
		DBHALOG_ERROR( "send heartbeat packet failed." );
	}

	m_heartbeat_timeout++;

	return true;
}

void CSyncClient::onStreamState( int type, EFS::TPacketBasePtr pkt )
{
	const char* strCnt[] = {"connected", "pktArrived", "conError", "closed"};
	int typeSize = sizeof( strCnt )/sizeof( char* );
	//DBHALOG_INFO("arrived %d %d",typeSize, type);
	if( type>=0 &&  type<=typeSize ){
		if( type==EFS::CStream::connected ){
			DBHALOG_INFO( "connected. state change to unsynced." );
			m_sync_state = unsynced; //连接上后,先要进行同步.
		}
		if( type==EFS::CStream::connError ){
			DBHALOG_ERROR( "connect error, to close." );
			m_close_mutex.enter();
			if( m_stream ){
				m_stream->close(); //回调函数中可以调用stream的close操作.
			}
			m_close_mutex.leave();
		}else if( type==EFS::CStream::closed ){
			DBHALOG_INFO( "connenct closed. state change to offline." );
			m_close_mutex.enter();
			ASSERT_ABORT( m_stream != NULL );
			delete m_stream;
			m_stream = NULL;
			if( m_close_sem ){
				m_close_sem->post(); 
			}
			m_sync_state = offline;
			m_heartbeat_timeout = 0;

			//如果有打开恢复文件,则关闭它.
			if( m_file.isOpen() ){
				m_file.close();
			}

			m_close_mutex.leave();
		}else if( type==EFS::CStream::pktArrived ){
			onPacket( pkt );
		}
	}else{
		DBHALOG_ERROR( "connect error state[%d]", type );
	}
}

// 获取当前备机上恢复文件，出参不带.recovery
void CSyncClient::getRecoveryFile(std::string& fileName, uint64_t& len)
{
	m_pre_recovery_file_name = "";
	m_pre_recovery_file_size = 0;

	fileName = "";
	len = 0;

	DIR *dir;
	dir=opendir(g_DBHAConfig->getSlaveBackupPath().c_str());
	if (dir == NULL){
		DBHALOG_ERROR("Open dir error.");
		if( ::mkdir(g_DBHAConfig->getSlaveBackupPath().c_str(), 0777) == -1 ){
			DBHALOG_ERROR( "mkdir dir(%s) failed.!\n", g_DBHAConfig->getSlaveBackupPath().c_str());
		}
		return;
	}

	struct dirent *ptr;
	// 在中途退出的时候，此处有可能会卡住
	while ((ptr=readdir(dir)) != NULL){
		if (m_exit){
			DBHALOG_ERROR("client is ready to exit.");
			break;
		}
		std::string file_name_tmp = ptr->d_name;
		string::size_type pos = file_name_tmp.find(recovery_file_suffix, 0);
		if (pos != std::string::npos){
			// 取出后缀.recovery
			m_pre_recovery_file_name = file_name_tmp.substr(0, pos);
			fileName = m_pre_recovery_file_name;

			std::string file_full_path = g_DBHAConfig->getSlaveBackupPath()+"/"+file_name_tmp;
			len = DBHA::CommonFun::getFileLen(file_full_path);
			m_pre_recovery_file_size = len;
			DBHALOG_INFO("recovery file pre name %s %s %lu.",
					file_name_tmp.c_str(), m_pre_recovery_file_name.c_str(), len);
			break;
		}
	}

	DBHALOG_INFO("recovery file pre name %s %lu.", m_pre_recovery_file_name.c_str(), len);

	closedir(dir);
}

EFS::TPacketBasePtr CSyncClient::onDumpFileInfoPacket( EFS::TPacketBasePtr msg )
{
	TResSyncDumpFileInfoMsgPtr res = dynamic_pointer_cast<CResSyncDumpFileInfoMessage>( newCResSyncDumpFileInfoMessage() );
	if( res==NULL ){
		DBHALOG_ERROR( "newCResSyncDumpFileInfoMessage failed, state change to syncError." );
		m_sync_state = syncError;
		return res;
	}

	std::string file_name = "";
	uint64_t len = 0;
	getRecoveryFile(file_name, len);

	//返回备机恢复文件信息.
	res->setDumpFileInfo(file_name, len);
	DBHALOG_INFO( "res send file info，name:%s, len:%lu", file_name.c_str(), len );
	return res;
}

EFS::TPacketBasePtr CSyncClient::onSyncBeginPacket( EFS::TPacketBasePtr msg )
{
	m_current_recovery_file = "";
	TResSyncBeginMsgPtr res = dynamic_pointer_cast<CResSyncBeginMessage>( newCResSyncBeginMessage() );
	if( res==NULL ){
		DBHALOG_ERROR( "newCResSyncBeginMessage failed, state change to syncError." );
		m_sync_state = syncError;
		return res;
	}

	//获取主机的dump文件.
	TSyncBeginMsgPtr req = dynamic_pointer_cast<CSyncBeginMessage>(msg);
	if(req==NULL){
		DBHALOG_ERROR( "CSyncBeginMessage failed, state change to syncError." );
		res->setErrorType( sqlGetFileNameFailed );
		m_sync_state = syncError;
		return res;
	}

	// 打开文件或者创建文件.
	string dump_file_name = "";
	uint64_t pos = 0;
	string md5sum = "";
	req->getFileInfo(dump_file_name, pos);
	req->getMd5sum(md5sum);
	if (dump_file_name == ""){
		DBHALOG_ERROR( "get dump tmp filename failed, state change to syncError." );
		res->setErrorType( sqlGetFileNameFailed );
		m_sync_state = syncError;
		return res;
	}
	// 当原始残留的恢复文件存在的时候
	if (m_pre_recovery_file_name != ""){
		if (dump_file_name != m_pre_recovery_file_name){
			// 如果文件不一样了，就将原始的文件删除
			std::string tmp_file = g_DBHAConfig->getSlaveBackupPath()+ "/"+
				m_pre_recovery_file_name + recovery_file_suffix;
			if( remove(tmp_file.c_str()) < 0 )
				DBHALOG_ERROR("remove old recovery file(%s) fail.",m_pre_recovery_file_name.c_str());
			else	
				DBHALOG_ERROR("remove old recovery file(%s) success.", m_pre_recovery_file_name.c_str());
		}else{
			if (pos > m_pre_recovery_file_size){
				DBHALOG_ERROR( "file size is error, master pos(%lu), slave len(%lu), state change to syncError.",
					pos, m_pre_recovery_file_size);
				res->setErrorType( sqlGetFileNameFailed );
				m_sync_state = syncError;
				return res;
			}
		}
	}

	// 构建新的恢复临时文件全路径
	m_current_recovery_file = g_DBHAConfig->getSlaveBackupPath()+ "/"+
		dump_file_name + recovery_file_suffix;
	m_current_recover_md5sum = md5sum;

	// DBHALOG_INFO("the slave recovery tmp dump tmp file name:%s",dump_file_full_path.c_str());
	//打开临时恢复文件.如果发生异常,文件关闭在断线时候关闭.
	if( !m_file.open( m_current_recovery_file.c_str(), Infra::CFile::modeWrite ) ){
		DBHALOG_ERROR( "open sql file[%s] failed, state change to syncError.", dump_file_name.c_str() );
		res->setErrorType( sqlOpenTmpFileFailed );
		m_sync_state = syncError;
		return res;
	}

	// 偏移到对应的位置进行
	m_file.seek((long)pos, Infra::CFile::begin);

	m_sync_state = inSyncing;
	DBHALOG_INFO( "begin sync dump file, state change to inSyncing, file name:%s, pos:%lu.",
			dump_file_name.c_str(), pos);

	return res;
}

EFS::TPacketBasePtr CSyncClient::onDataSyncPacket( EFS::TPacketBasePtr msg )
{
	TResSyncDataMsgPtr res = dynamic_pointer_cast<CResSyncDataMessage>( newCResSyncDataMessage() );
	if( res==NULL ){
		DBHALOG_ERROR( "newCResSyncDataMessage failed, state change to syncError." );
		m_sync_state = syncError;
		return res;
	}
	
	TSyncDataMsgPtr req = dynamic_pointer_cast<CSyncDataMessage>( msg );
	if( !m_file.isOpen() ){
		DBHALOG_ERROR( "file is not opened, state change to syncError." );
		res->setErrorType( sqlTmpFileError );
		m_sync_state = syncError;
		return res;
	}

	void* data = NULL;
	int32_t data_len = 0;
	req->getSyncMessage( ( char** )&data, data_len );
	if( data_len==0 ){
		DBHALOG_WARN( "get datalen NULL." );
		return res;
	}

	if( m_file.write( data, data_len )<0 ){
		DBHALOG_ERROR( "write sql tmp file failed, state change to syncError." );
		res->setErrorType( sqlTmpFileError );
		m_sync_state = syncError;
		return res;
	}
	
	return res;
}

EFS::TPacketBasePtr CSyncClient::onSyncEndPacket( EFS::TPacketBasePtr msg )
{
	//创建返回包.
	TSyncEndMsgPtr req = dynamic_pointer_cast<CSyncEndMessage>(msg);
	TResSyncEndMsgPtr res = dynamic_pointer_cast<CResSyncEndMessage>( newCResSyncEndMessage() );
	if( res==NULL ){
		DBHALOG_ERROR( "newCResSyncEndMessage failed, state change to syncError." );
		m_sync_state = syncError;
		return res;
	}
	req->getBinlogInfo(m_current_recover_binlog_file, m_current_recover_binlog_pos);

	//关闭在写恢复文件.
	m_file.close();

	//获取文件名.
	//todo需要改动
	std::string dump_file_name = g_DBHAConfig->getSlaveBackupPath()+"/"+
		g_DBHAConfig->getSlaveRecoverDumpFileName();
	//std::string dumpTmpFileName = g_DBHAConfig->getSlaveBackupPath()+"/"+
	//	g_DBHAConfig->getSlaveRecoverDumpFileName()+".tmp";//todo
	if( dump_file_name.empty() || m_current_recovery_file.empty() ){
		DBHALOG_ERROR( "get dump filename failed, state change to syncError." );
		res->setErrorType( sqlGetFileNameFailed );
		m_sync_state = syncError;
		return res;
	}

	//将临时文件重命名为新的数据库文件.
	if( !Infra::CFile::rename( m_current_recovery_file.c_str(), dump_file_name.c_str() ) ){
		DBHALOG_ERROR( "rename dump tmp file failed, state change to syncError." );
		res->setErrorType( renameFileFailed );
		m_sync_state = syncError;
		return res;
	}

	//校验MD5值
	if( true != checkRecoverFileMd5sum(dump_file_name, m_current_recover_md5sum) ){
		res->setErrorType( dumpFileMd5CheckFailed );
	}

	wakeupDumping();
	m_sync_state = synced;
	//释放锁.
	//文件传输完开始恢复
	//DBHALOG_INFO( "sync dump file success, state change to synced,state=%d .",(int32_t)m_sync_state );
	
	return res;
}

//给定恢复文件全路径校验恢复文件的md5值
bool CSyncClient::checkRecoverFileMd5sum(std::string dump_file_name, std::string& md5sum)
{
	//md5值校验
	char command[2048] = { 0 };
	if( snprintf( command, 2048, "md5sum  %s", dump_file_name.c_str() ) < 0 ) {
		DBHALOG_ERROR( "snprintf failed!\n");
		return false;
	}

	std::stringstream ret_value;
	int32_t ret = Dahua::EFS::os::shell( &ret_value, std::string( command ) );
	if ( ret ) {
		DBHALOG_ERROR( "excute command: %s failed!\n", command );
		return false;
	}

	char md5[256],dump[256];
	if(sscanf(ret_value.str().c_str(), "%s %s", md5, dump) != 2){
		DBHALOG_ERROR( "parser version failed %s.\n", ret_value.str().c_str());
		return false;
	}
	int cmpRet = strcmp(md5, md5sum.c_str());
	if ( cmpRet == 0 ){
		DBHALOG_INFO( "file:%s (%s) (%s) [%d]\n", dump_file_name.c_str(), md5, md5sum.c_str(), cmpRet);
		return true;
	}else{
		DBHALOG_ERROR( "file:%s (%s) (%s) [%d] check failed!\n",
				dump_file_name.c_str(), md5, md5sum.c_str(), cmpRet);
		return false;
	}
}

void CSyncClient::onPacket( EFS::TPacketBasePtr pkt )
{
	if( pkt==NULL ){
		DBHALOG_ERROR( "recv invalied packet." );
		return;
	}

	m_heartbeat_timeout = 0; //收到任何包都认为连接没有问题.

	if( m_sync_state==syncError ){
		DBHALOG_ERROR( "state is error, will close stream." );
		return;
	}

	uint8_t pkt_type = pkt->getPktType();
	uint16_t pkt_cmd = pkt->getPktCmd();
	//DBHALOG_INFO( "type %d cmd %d,",pkt_type, pkt_cmd );
	if( pkt_type == EFS::DS_PROTOCOL_REQ ){

		EFS::TPacketBasePtr retPkt;

		switch( pkt_cmd )
		{
		case HAFK_SYNC_DUMP_FILE_INFO_MESSAGE:
			retPkt = onDumpFileInfoPacket( pkt );
			break;
		case HAFK_SYNC_BEGIN_MESSAGE:
			retPkt = onSyncBeginPacket( pkt );
			break;
		case HAFK_SYNC_DATA_MESSAGE:
			retPkt = onDataSyncPacket( pkt );
			break;
		case HAFK_SYNC_END_MESSAGE:
			retPkt = onSyncEndPacket( pkt );
			break;
			break;
		case HAFK_HEARTBEAT_MESSAGE:
			DBHALOG_INFO( "heartbeat response." );
			break;
		default:
			DBHALOG_ERROR( "invald packet type[%d].", pkt_cmd );
			return;
		}

		if( retPkt!=NULL ){
			if( m_stream->send( retPkt )<0 ){
				DBHALOG_ERROR( "send packet[%d,%d] failed.", retPkt->getPktCmd(), retPkt->getPktType() );
			}
		}
	}
}

int32_t CSyncClient::getSyncState()
{
	return m_sync_state;
}


} // DBHA
} // VideoCloud
} // Dahua

