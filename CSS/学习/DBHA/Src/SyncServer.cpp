#include "SyncServer.h"
#include "HAConfigs.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/NetThread.h"
#include "MessageType.h"
#include "SyncBeginMessage.h"
#include "SyncDataMessage.h"
#include "SyncEndMessage.h"
#include "SyncDumpFileInfoMessage.h"
#include "HAHeartbeatMessage.h"
#include "HALog.h"
#include "HAErrorCode.h"
#include "Infra/Guard.h"
#include "Infra/File.h"
#include "Common/Defs.h"
#include "DBHAConfig.h"
#include "Common/os.h"
#include "CommonFun.h"

namespace Dahua {
namespace VideoCloud { 
namespace DBHA {

using namespace std;
DBHALOG_CLASSNAME( CSyncServer );

const int32_t READ_SIZE = 1024*512;
CSyncServer::CSyncServer( ) 
: CThread( "sync thread" )
{
	m_is_started = false;
	m_is_init_server = false;
	m_connect_ptr = NULL;
	m_close_sem = NULL;
	m_wait_sem = NULL;
	m_sync_state = noSync;
	m_is_syncing = false;
	m_cmd_type = 0;
	m_is_error = 0;
	m_error_type = 0;
	//m_have_datanode_module = false;
	m_connect = false;
	m_slave_recovery_file_len = 0;
	m_master_dump_file_pos = 0;
	m_pre_master_dump_file_name = "";
	m_pre_dump_file_cnt = 0;
	m_master_dump_binlog_file="";
	m_master_dump_binlog_pos = 0;
}

CSyncServer::~CSyncServer( void )
{
	if( m_is_started ){
		DBHALOG_ERROR( "sync server is not stoped before destory it." );
		cleanup();
	}

	if( m_is_init_server ){
		CServer::close();
		m_is_init_server = false;
	}
}

bool CSyncServer::startup()
{
	if( m_is_started ){
		DBHALOG_ERROR( "sync server is started, return failed." );
		return false;
	}

	if( !m_is_init_server ){
		NetFramework::CSockAddrIPv4 addr( "0.0.0.0", gHAConfig.getSyncServerPort() );
		if( CServer::init( addr )<0 ){
			DBHALOG_ERROR( "sync server[%s,%d] init failed.", "0.0.0.0", gHAConfig.getSyncServerPort() );
			return false;
		}
		m_is_init_server = true;
	}

	m_is_started = true;
	//m_have_datanode_module = haveDataNodeModule;
	DBHALOG_INFO( "sync server is start success." );

	return true;
}

bool CSyncServer::cleanup()
{
	DBHALOG_INFO( "CSyncServer clean up begin." );
	if( !m_is_started ){
		DBHALOG_INFO( "sync server is not started." );
		return true;
	}

	m_connect_mutex.enter();
	if( !m_connect_ptr ){
		m_is_started = false;
		m_connect_mutex.leave();
		DBHALOG_ERROR( "sync server is stop success." );
		return true;
	}

	m_close_sem = new Infra::CSemaphore();
	ASSERT_ABORT( m_close_sem!=NULL );

	m_connect_ptr->close();
	m_connect_mutex.leave();

	m_close_sem->pend();

	m_connect_mutex.enter();
	delete m_close_sem;
	m_close_sem = NULL;
	m_connect_mutex.leave();


	m_is_started = false;

	DBHALOG_INFO( "sync server is stop success." );
	return true;
}

int CSyncServer::accept( NetFramework::CSockStream& sock )
{	
	if( !m_is_started ){
		DBHALOG_ERROR( "server is not started." );
		return -1;
	}

	Infra::CGuard guard( m_connect_mutex ); 

	if( m_connect_ptr ){
		DBHALOG_ERROR( "sync connect is existed." );
		return -1;
	}

	CSyncConnect::CloseProc closeProc( &CSyncServer::closeConnect, this );
	CSyncConnect::PacketProc pktProc( &CSyncServer::onPacket, this );
	CSyncConnect* connect = new CSyncConnect( closeProc, pktProc );
	if( !connect ){
		DBHALOG_ERROR( "alloc CSyncConnect failed." );
		return -1;
	}

	if( !connect->init( sock ) ){
		DBHALOG_ERROR( "sync connect init failed." );
		delete connect;
		return -1;
	}

	m_connect_ptr = connect;
	m_connect = true;

	createThread();
	return 0;
}

void CSyncServer::closeConnect()
{
	m_connect = false;
	if(m_error_type != dumpFileMd5CheckFailed){
		stopMysqlDump();
	}

	destroyThread();

	DBHALOG_ERROR( "connenct closed begin." );
	m_connect_mutex.enter();
	ASSERT_ABORT( m_connect_ptr != NULL );
	delete m_connect_ptr;
	m_connect_ptr = NULL;
	if( m_close_sem ){
		m_close_sem->post();
	}
	m_sync_state=noSync;
	m_connect_mutex.leave();
	DBHALOG_ERROR( "connenct closed end!" );
}

void CSyncServer::onPacket( EFS::TPacketBasePtr req, EFS::TPacketBasePtr& rsp )
{
	uint16_t pkt_cmd = req->getPktCmd();
	uint8_t pkt_type = req->getPktType();
	if( pkt_type==EFS::DS_PROTOCOL_REQ ){
		if( pkt_cmd==HAFK_HEARTBEAT_MESSAGE ){
			//心跳模块有过滤过uid,所以这里不再比较uid.
			//心跳回应包也采用请求包的形式,因为SyncClient.
			//采用收到任何包都认为没有断线的方式.方便进行.
			//心跳超时的计数.
			rsp = newCHAHeartbeatMessage();
		}else{
			DBHALOG_ERROR( "invald packet type[%d].", pkt_cmd );
		}
	}else if( pkt_type==EFS::DS_PROTOCOL_RES ){
		Infra::CGuard guard( m_connect_mutex );
		if( m_cmd_type==pkt_cmd ){
			if( pkt_cmd==HAFK_SYNC_DATA_MESSAGE ){
				TResSyncDataMsgPtr ptr = dynamic_pointer_cast<CResSyncDataMessage>( req );
				m_is_error = ptr->isError();
				m_error_type = ptr->getErrorType();
			}else if( pkt_cmd==HAFK_SYNC_BEGIN_MESSAGE ){
				TResSyncBeginMsgPtr ptr = dynamic_pointer_cast<CResSyncBeginMessage>( req );
				m_is_error = ptr->isError();
				m_error_type = ptr->getErrorType();
			}else if( pkt_cmd==HAFK_SYNC_END_MESSAGE ){
				TResSyncEndMsgPtr ptr = dynamic_pointer_cast<CResSyncEndMessage>( req );
				m_is_error = ptr->isError();
				m_error_type = ptr->getErrorType();
			}else if( pkt_cmd==HAFK_SYNC_DUMP_FILE_INFO_MESSAGE ){
				TResSyncDumpFileInfoMsgPtr ptr = dynamic_pointer_cast<CResSyncDumpFileInfoMessage>( req );
				m_is_error = ptr->isError();
				m_error_type = ptr->getErrorType();
				ptr->getDumpFileInfo(m_slave_recovery_file_name, m_slave_recovery_file_len);
				DBHALOG_INFO( "m_slave_recovery_file_name:%s.", m_slave_recovery_file_name.c_str() );
			}else{
				DBHALOG_ERROR( "invald packet type[%d].", pkt_cmd );
				return;
			}
			
			//释放同步等待信号.
			if( m_wait_sem ){
				m_wait_sem->post();
			}
		}else{
			DBHALOG_ERROR( "invald packet type[%d].", pkt_cmd );
		}
	}
}


bool CSyncServer::sendToSlave( EFS::TPacketBasePtr req )
{
	m_connect_mutex.enter(); //保证send时候不会被delete,以及信号量被安全创建出来.

	if( m_connect_ptr==NULL ){ 
		//没有备机直接返回.
		m_cmd_type = req->getPktCmd();
		m_is_error = 1;
		m_error_type = noSlaveOnline;
	}else{
		if( m_sync_state==noSync && req->getPktCmd()==HAFK_JSON_MESSAGE ){
			m_is_error = 1;
			m_error_type = sqlUnsynced;
		}else{
			if( m_connect_ptr->send( req )<0 ){
				m_cmd_type = 0;
				m_is_error = 1;
				m_error_type = sendToSlaveFailed;
			}else{
				m_is_error = 0;
				m_cmd_type = req->getPktCmd();
			}
		}
	}

	return true;
}

bool CSyncServer::waitSlaveResponse( uint32_t timeout )
{
	//有错误直接可以返回.
	if( m_is_error ){
		DBHALOG_ERROR( "send to slave error[%d].", m_error_type );
		m_connect_mutex.leave();
		return false;
	}

	//等待响应.每次都需要新的信号量,以免超时后在发生信号量的post,从而发生计数错误问题.
	ASSERT_ABORT( m_wait_sem==NULL );
	m_wait_sem = new Infra::CSemaphore();
	ASSERT_ABORT( m_wait_sem!=NULL );
	m_connect_mutex.leave();
	int ret = m_wait_sem->pend( timeout );
	m_connect_mutex.enter();
	delete m_wait_sem;
	m_wait_sem = NULL;
	m_cmd_type = 0;
	
	//等待超时.
	if( ret == -1 ){
		m_is_error = 1;
		m_error_type = waitSlaveResposeTimeout;
	}

	if(m_error_type == dumpFileMd5CheckFailed){
		//接收到备机的备份文件MD5验证失败消息，重新生成备份文件
		if(true == checkmysqlMasterDump()){
			DBHALOG_INFO( "the Backup is runing ,not start again." );
		}else{
			mysqlMasterDump();
			DBHALOG_INFO( "md5 check the dump file failed,recreate master dump file.");
		}
	}

	//消息返回错误.
	if( m_is_error ){
		//发生错误关闭链接.
		if( m_connect_ptr ){
			m_connect_ptr->close();
		}
		m_connect_mutex.leave();
		DBHALOG_ERROR( "waitslave response[%d] failed with error[%d].", m_cmd_type, m_error_type );
		return false;
	}

	m_connect_mutex.leave();

	return true;
}

void CSyncServer::threadProc()
{
	DBHALOG_INFO( ".........start sync........." );

	bool ret = true;
	//todo此处需要判断dump文件的完整性
	m_is_syncing = true;

	do 
	{
		/*
		 * 如果是扩容模式，首先产生一个最新的本机备份，避免上一次的备份文件和当前文件数据相差过大导致扩容结束后;
		 * 备机还未同步完成就主备切换会导致数据丢失
		 */
		if( g_DBHAConfig->serviceInExpandMode() ){
			DBHALOG_INFO( "generate real-time backup begin,maybe consume much time..." );
			if( !mysqlMasterDump() ){
				DBHALOG_ERROR( "generate real-time backup file failed,use old backup file..." );
			}else{
				DBHALOG_INFO( "generate real-time backup file success..." );
			}
		}

		//获取备机的恢复文件信息
		if( !getDumpFileInfo() ){
			DBHALOG_ERROR( "get slave dump file failed." );
			ret = false;
			break;
		}
		//开始同步.
		if( !beginSync() ){
			DBHALOG_ERROR( "begin sync failed." );
			ret = false;
			break;
		}

		//发送文件.
		bool post_file_ret = postFile();

		//操作都成功了变为已同步状态.
		if( post_file_ret ){
			m_sync_state = synced;
			DBHALOG_INFO( ".........sync success........." );
		}else{
			DBHALOG_INFO( ".........sync failed........." );
			ret = false;
		}
	} while (0);

	if (!ret){
		m_connect_mutex.enter();
		if( m_connect_ptr )
			m_connect_ptr->close();

		m_connect_mutex.leave();
		m_is_syncing = false;
	}
}

bool CSyncServer::getDumpFileInfo()
{
	m_slave_recovery_file_name = "";
	m_slave_recovery_file_len = 0;
	m_master_dump_file_name = "";
	m_master_dump_full_file = "";
	TSyncDumpFileInfoMsgPtr ptr = dynamic_pointer_cast<CSyncDumpFileInfoMessage>( newCSyncDumpFileInfoMessage() );
	//ptr->setDumFileInfo(gHAConfig.getSyncVersion());
	sendToSlave( ptr );
	if( !waitSlaveResponse( gHAConfig.getSyncCallTimeout() ) ){
		return false;
	}

	// 根据备机的信息获取主机上的信息
	int32_t ret = getMasterFile();
	// 如果主机获取失败，则自动手动产生一个备份文件
	bool error_tmp = false;
	if (m_master_dump_full_file == ""){
		if (!mysqlMasterDump()){
			error_tmp = true;
		}else{
			ret = getMasterFile();
			if (m_master_dump_full_file == "")
				error_tmp = true;
		}
	}

	if (error_tmp){
		//发生错误关闭链接.
		m_connect_mutex.enter();
		if( m_connect_ptr ){
			m_connect_ptr->close();
		}
		m_connect_mutex.leave();
		DBHALOG_ERROR( "close connect");
		return false;
	}
	//parseDumpInfo内为m_master_dump_file_md5赋值
	return parseDumpInfo(!((bool)ret)); 
}

// 根据备机文件获取一个主机的备份文件，1、主备是一样的文件，且非空;2:主备文件不一样；3主备都是空的
int32_t CSyncServer::getMasterFile()
{
	m_master_dump_full_file = "";
	char command[2048] = { 0 };
	if( snprintf( command, 2048, "/bin/sh  %s %s %s %s %s %s", (g_DBHAConfig->opMasterBackupFile()).c_str() ,
		"getFile", g_DBHAConfig->getMasterBackupPath().c_str(), (gHAConfig.getUId()).c_str(),
		m_server_id.c_str(), m_slave_recovery_file_name.c_str()) < 0 ) {
			DBHALOG_ERROR( "getMasterFile failed!\n" );
			return false;
	}
	std::stringstream dump_file;
	int32_t ret = Dahua::EFS::os::shell( &dump_file, std::string( command ) );
	std::string::size_type loc = dump_file.str().find("\n", 0);
	if (loc == std::string::npos){
		DBHALOG_INFO( "get dump_file(%s) failed, cmd:%s\n", dump_file.str().c_str(), command);
		m_master_dump_full_file = "";
		return false;
	}

	//返回结果应为
	StringVec itemvec;
	string dump_source = dump_file.str();
	char full_file[256], binlog_file[256], md5[256];
	int binlog_pos;
	if( sscanf(dump_source.c_str(), "%s %s %d %s", full_file, binlog_file, &binlog_pos, md5) != 4){
		DBHALOG_INFO( "parser dump file failed %s.\n", dump_source.c_str());
		m_master_dump_full_file = "";
		return false;
	}

	m_master_dump_full_file = full_file;
	m_master_dump_binlog_pos = binlog_pos;
	m_master_dump_binlog_file = binlog_file;
	m_master_dump_md5sum = md5;
	m_master_dump_binlog_file = m_master_dump_binlog_file.substr(m_master_dump_binlog_file.find_last_of("/") + 1);
	std::string tmp_dump_file= m_master_dump_full_file.substr(m_master_dump_full_file.find_last_of("/") + 1);
	DBHALOG_INFO( "master_file(%s) binlog(%s) pos(%lld) md5(%s) success,cmd:%s\n",
					tmp_dump_file.c_str(), m_master_dump_binlog_file.c_str(),
					(long long)m_master_dump_binlog_pos, m_master_dump_md5sum.c_str(), command);
	return ret; // ret=0表示两者肯定相同，且肯定不为空
}

bool CSyncServer::cleanMasterFile()
{
	m_master_dump_full_file = "";
	char command[2048] = { 0 };
	if( snprintf( command, 2048, "cleanFile %s %s %s %s", g_DBHAConfig->getMasterBackupPath().c_str(),
		(gHAConfig.getUId()).c_str(), m_server_id.c_str(),m_pre_master_dump_file_name.c_str()) < 0 ) {
			DBHALOG_ERROR( "%s %d snprintf failed!\n",__FUNCTION__, __LINE__ );
			return false;
	}

	bool ret = CommonFun::executeScript((g_DBHAConfig->opMasterBackupFile()).c_str(), command);
	if ( !ret ) {
		DBHALOG_ERROR( "cleanup master backup file failed." );
		return false;
	}

	DBHALOG_INFO( "cleanup master backup file success." );
	return true;
}

// 主机上的dump文件信息，flag表示主机的备份文件是否和备机一一致
bool CSyncServer::parseDumpInfo( bool flag)
{
	if (m_master_dump_full_file == ""){
		DBHALOG_ERROR("m_master_dump_full_file is empty.");
		return false;
	}
	// 根据备机的恢复文件找主机上的备份文件
	std::string::size_type pos = m_master_dump_full_file.find_last_of("/");
	if (pos == std::string::npos){
		DBHALOG_INFO( "parse master_file(%s) failed\n", m_master_dump_full_file.c_str());
		return false;
	}

	m_master_dump_file_name = m_master_dump_full_file.substr(pos+1, string::npos);
	if (flag){ // 断点续传
		assert(m_slave_recovery_file_name == m_master_dump_file_name);
		m_master_dump_file_pos = m_slave_recovery_file_len;
	}else{    // 重新传文件
		assert(m_slave_recovery_file_name != m_master_dump_file_name);
		m_master_dump_file_pos = 0;

		// 判定当前获取到的文件是否已经被重复使用过n次进行恢复了,直接删除
		if ((m_pre_master_dump_file_name == m_master_dump_file_name) 
			&& (m_pre_dump_file_cnt > g_DBHAConfig->getDumpFileValidNum())){
			// 超过一定次数，则直接不需要该备份文件
			cleanMasterFile();
		}
	}
	// ret:0表示两者是一样的，1表示两者是不一样的
	DBHALOG_INFO("slave recovery info:%s %lu, master dump info:%s %lu, flag:%d.",
				m_slave_recovery_file_name.c_str(),
				m_slave_recovery_file_len, m_master_dump_file_name.c_str(),
				m_master_dump_file_pos, flag);
	return true;
}

//检测后台是否已经在备份 true-在备份，false-不再或执行失败
bool CSyncServer::checkmysqlMasterDump()
{
	m_master_dump_full_file = "";
	char command[2048] = { 0 };
	if( snprintf( command, 2048, "checkDump %s %s %s", g_DBHAConfig->getMasterBackupPath().c_str(),
		(gHAConfig.getUId()).c_str(), m_server_id.c_str()) < 0 ) {
			DBHALOG_ERROR( "%s %d snprintf failed!\n",__FUNCTION__, __LINE__ );
			return false;
	}

	bool ret = CommonFun::executeScript((g_DBHAConfig->opMasterBackupFile()).c_str(), command);
	if ( !ret ) {
		DBHALOG_ERROR( "checkmysqlMasterDump failed." );
		return false;
	}

	DBHALOG_INFO( "checkmysqlMasterDump success." );
	return true;
}


bool CSyncServer::mysqlMasterDump()
{
	char para[10240] = {0};
	snprintf(para, 10240, "start %s %s %s master",g_DBHAConfig->getMasterBackupPath().c_str(), gHAConfig.getUId().c_str() ,
		m_server_id.c_str());

	bool ret = CommonFun::executeScript(g_DBHAConfig->getBackupScript(), std::string(para));
	if (!ret) {
		DBHALOG_ERROR( "mysqlMasterDump fail." );
		return false;
	}
	DBHALOG_INFO( "mysqlMasterDump success." );
	return true;
}

bool CSyncServer::stopMysqlDump()
{
	char para[10240] = {0};
	snprintf(para, 10240, "stop %s %s %s master",g_DBHAConfig->getMasterBackupPath().c_str(), gHAConfig.getUId().c_str() ,
		m_server_id.c_str());

	bool ret = CommonFun::executeScript(g_DBHAConfig->getBackupScript(), std::string(para));
	if (!ret) {
		DBHALOG_ERROR( "stop mysql dump fail." );
		return false;
	}
	DBHALOG_INFO( "stop mysqlMasterDump success." );
	return true;
}

bool CSyncServer::beginSync()
{
	TSyncBeginMsgPtr ptr = dynamic_pointer_cast<CSyncBeginMessage>( newCSyncBeginMessage() );
	DBHALOG_INFO("master dump:%s,file_pos:%lu.", m_master_dump_file_name.c_str(), m_master_dump_file_pos);
	//开始添加md5内容
	ptr->setFileInfo(m_master_dump_file_name, m_master_dump_file_pos);
	ptr->setMd5sum(m_master_dump_md5sum);
	sendToSlave( ptr );
	if( !waitSlaveResponse( gHAConfig.getSyncCallTimeout() ) ){
		return false;
	}

	return true;
}

bool CSyncServer::postData( char* data, int32_t len )
{
	TSyncDataMsgPtr ptr = dynamic_pointer_cast<CSyncDataMessage>( newCSyncDataMessage() );
	ptr->setSyncMessage( data, len );
	sendToSlave( ptr );
	if( !waitSlaveResponse( gHAConfig.getSyncCallTimeout() ) ){
		return false;
	}

	return true;
}

bool CSyncServer::postFile()
{
	if (m_master_dump_file_name == ""){
		DBHALOG_ERROR( "sync open file[%s] failed.", m_master_dump_file_name.c_str() );
		return false;
	}

	char* data_buf = new char[READ_SIZE];
	if( !data_buf ){
		DBHALOG_ERROR( "alloc data buffer failed." );
		return false;
	}

	DBHALOG_INFO("m_master_dump_full_file : %s, pos : %lu.", m_master_dump_full_file.c_str(), m_master_dump_file_pos);
	//打开备份文件.
	Infra::CFile file;
	if( !file.open( m_master_dump_full_file.c_str(), Infra::CFile::modeRead|Infra::CFile::typeBinary ) ){
		DBHALOG_ERROR( "sync open full file[%s], file name[%s] failed.", m_master_dump_full_file.c_str(), m_master_dump_file_name.c_str() );
		delete []data_buf;
		return false;
	}

	file.seek((long)m_master_dump_file_pos, Infra::CFile::begin);

	//发送数据.
	int32_t read_size = 0;
	while( ( read_size = file.read( data_buf, READ_SIZE ) ) > 0 ){
		if (!m_connect){
			DBHALOG_ERROR("connect is closed.");
			delete []data_buf;
			file.close();
			return false;
		}
		if( !postData( data_buf, read_size ) ){
			DBHALOG_ERROR( "post data to slave failed." );
			delete []data_buf;
			file.close();
			return false;
		}
	}

	delete []data_buf;
	file.close();

	if( !endSync() ){
		DBHALOG_ERROR( "end sync failed." );
		return false;
	}

	if( g_DBHAConfig->serviceInExpandMode() ){
		g_DBHAConfig->serviceLeavingExpandMode();
		DBHALOG_INFO( "service leave expand mode now...\n" );
	}

	if (m_pre_master_dump_file_name != m_master_dump_file_name){
		m_pre_master_dump_file_name = m_master_dump_file_name;
		m_pre_dump_file_cnt = 0;
	}else{
		m_pre_dump_file_cnt++;
	}

	return true;
}

bool CSyncServer::endSync()
{
	TSyncEndMsgPtr ptr = dynamic_pointer_cast<CSyncEndMessage>( newCSyncEndMessage() );
	ptr->setBinlogInfo(m_master_dump_binlog_file, m_master_dump_binlog_pos);
	sendToSlave( ptr );
	if( !waitSlaveResponse( gHAConfig.getSyncCallTimeout() ) ){
		return false;
	}

	return true;
}

bool CSyncServer::isSyncing()
{
	return m_is_syncing;
}

void CSyncServer::setMysqlServerID(std::string serverID)
{
	m_server_id = serverID;
}
}// DBHA
} // VideoCloud
} // Dahua

