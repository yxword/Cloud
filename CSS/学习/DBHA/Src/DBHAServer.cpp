#include <stdlib.h>
#include <iostream>
#include "DBHAServer.h"
#include "HAConfigs.h"
#include "HALog.h"
#include "Common/os.h"
#include "CommonFun.h"

using namespace Dahua::Infra;
extern Dahua::Infra::CSemaphore gExitSem;
extern bool g_exit;

Dahua::VideoCloud::DBHA::MasterBinlogInfo gHeartBeatBinlogInfo;

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

using namespace std;

DBHALOG_CLASSNAME( CDBHAServer );

string DBHAFrameworkDir = "../Config/";//数据库监控程序配置文件夹
const int slaveNotSyncNum=10;
int recovery_retry_count=0;

CDBHAServer::CDBHAServer()
:m_is_master(false)
,m_slave_is_online(false)
,m_check_db_timer("check_slave_timer")
,m_server_id("")
,m_thread(NULL)
,m_ha_waiting_for_recovery(NULL)
,m_check_error_num(0)
,m_mysql_sync_ip("")
,m_last_check_binlog_time(0)
{
	m_link_status = 0;
}

CDBHAServer::~CDBHAServer()
{

}

bool CDBHAServer::setWorkspace(std::string& path)
{
	// 设置路径
	gHAConfig.setWorkspace( path );

	if ( gHAConfig.NotSlaveNotMaster != gHAConfig.getSQLMode() )
	{
		DBHALOG_INFO("the init mysql mode is not no slave or no master,need clear history mode.");
		gHAConfig.setSQLMode(gHAConfig.NotSlaveNotMaster);
	}
	// 导入haconfig信息
	gHAConfig.loadConfig();

	// 如果是全新机器，则写入init信息
	if (gHAConfig.newMachine()){
		// 尝试写入binlog的init位置
		string initBinlogInfo = "";
		bool ret = CommonFun::getExecuteScriptValue(\
			CDBHAConfig::instance()->getBinlogInfoScript(), \
			"",initBinlogInfo);
		if (!ret)
			DBHALOG_ERROR("set init binlog failed");
		else
			gHAConfig.setInitBinlogInfo(initBinlogInfo);

	}

	// 导入ha state信息
	gHAConfig.loadHAState();
	// 导入mysql state信息
	gHAConfig.loadSqlState();

	return true;
}

bool CDBHAServer::getDBHAConfig(HAConfig& cfg)
{
	cfg.multicastIP = g_DBHAConfig->getMulticastIP();
	cfg.multicastPort = g_DBHAConfig->getMulticastPort();
	cfg.syncPort = g_DBHAConfig->getSyncPort();
	cfg.vifName = g_DBHAConfig->getVifName();
	cfg.ifName = g_DBHAConfig->getIfName();
	cfg.heartbeatifName = g_DBHAConfig->getHeartbeatIfName();
	cfg.serviceName = g_DBHAConfig->getServiceName();
	cfg.userDefined = g_DBHAConfig->getUserDefined();

	cfg.localHeartbeatIP = g_DBHAConfig->getLocalHeartbeatIP();
	cfg.peerHeartbeatIP = g_DBHAConfig->getPeerHeartbeatIP();
	cfg.localBusinessIP = g_DBHAConfig->getLocalBusinessIP();
	cfg.peerBusinessIP = g_DBHAConfig->getPeerBusinessIP();
	cfg.vip = g_DBHAConfig->getVIP();
	cfg.vipNetmask = g_DBHAConfig->getVIPNetmask();

	// 针对第一次配置，hold住.当对端是空白的时候，默认就是单机启动
	if ( cfg.localBusinessIP == ""
		|| cfg.vip == "" || cfg.vipNetmask == "" ) {

			gExitSem.pend();
	} 

	cfg.localHeartbeatIP = g_DBHAConfig->getLocalHeartbeatIP();
	cfg.peerHeartbeatIP = g_DBHAConfig->getPeerHeartbeatIP();
	cfg.localBusinessIP = g_DBHAConfig->getLocalBusinessIP();
	cfg.peerBusinessIP = g_DBHAConfig->getPeerBusinessIP();
	cfg.vip = g_DBHAConfig->getVIP();
	cfg.vipNetmask = g_DBHAConfig->getVIPNetmask();

	if ( cfg.localBusinessIP == ""
		|| cfg.vip == "" || cfg.vipNetmask == "" ) {
			DBHALOG_ERROR( "cfg's element value is NULL or recieve Ctrl+C. " );
			return false;
	} 

	// 在主备双机的时候，两者对应的心跳ip必须要有
	if (cfg.localBusinessIP != "" &&  cfg.peerBusinessIP != "" ){
		if (cfg.localHeartbeatIP == "" || cfg.peerHeartbeatIP == ""){
			DBHALOG_ERROR( "localBusinessIP:%s, peerBusinessIP:%s,localHeartbeatIP:%s,peerHeartbeatIP:%s",
				cfg.localBusinessIP.c_str(), cfg.peerBusinessIP.c_str(), cfg.localHeartbeatIP.c_str(),
				cfg.peerHeartbeatIP.c_str());
			return false;
		}
	}

	return true;
}

bool CDBHAServer::setHAConfig(HAConfig& cfg)
{
	//获取mysql服务主备的server-id
	//目前采用的策略是心跳IP的最后一位作为server-id
	getDBServerID(cfg.localHeartbeatIP,m_server_id);

	bool ret = CommonFun::executeScript(CDBHAConfig::instance()->getSetServerIDScript(), m_server_id);
	if ( !ret ) {
		DBHALOG_ERROR( "set server id fail." );
		return false;
	}
	//syncserver设置mysqlserverID用于获取主机的dump文件
	m_sync_server.setMysqlServerID(m_server_id);
	gHAConfig.setHAConfig( cfg );
	return true;
}

uint8_t CDBHAServer::getClusterID()
{
	return gHAConfig.getClusterId();
}


bool CDBHAServer::clearClusterID()
{
	gHAConfig.setClusterId( 0 );
	gHAConfig.setUId( "" );
	gHAConfig.saveConfig();
	return true;
}


bool CDBHAServer::init()
{
	setWorkspace(DBHAFrameworkDir);
	
	HAConfig cfg;
	bool ret = getDBHAConfig( cfg );

	if (!ret)
	{
		DBHALOG_ERROR("get DBHA config fail");
		return false;
	}

	ret = setHAConfig(cfg);
	if (!ret)
		DBHALOG_ERROR("set ha config failed");

	return true;
}

bool CDBHAServer::startup()
{
	JudgerMap judgers;
	CHeartbeatMonitor::Judger master_judger( &CDBHAServer::masterJudger, this );
	judgers[CHeartbeatMonitor::BECOME_MASTER].push_back( master_judger );
	CHeartbeatMonitor::Judger fast_master_judger( &CDBHAServer::fastMasterJudger, this );
	judgers[CHeartbeatMonitor::FAST_BECOME_MASTER].push_back( fast_master_judger );
	
	CHeartbeatMonitor::HAStateProc3 proc( &CDBHAServer::haStateChange, this );
	bool ret = m_monitor.startup( proc, judgers );
	if (!ret) 
		return false;

	DBHALOG_INFO( "DBHAFramework startup return %d.", ret );
	//ret = m_check_mysql_service_timer.start( CTimer::Proc( &CDBHAServer::checkMysqlServiceProc, this ),
	//	0, g_DBHAConfig->getCheckMysqlServicePeriod() );
	//if ( !ret ) {
	//	DBHALOG_ERROR( "start m_check_mysql_service_timer fail" );
	//}
	//else
	//{
	//	DBHALOG_INFO( "start m_check_mysql_service_timer success." );
	//}
	return ret;
}

bool CDBHAServer::cleanup()
{
	m_monitor.cleanup();
	m_sync_server.cleanup();
	//将主备机的当前状态置为-1
	if ( -1 != gHAConfig.getSQLMode() )
	{
		DBHALOG_INFO("stop DBHAServer,mysql mode is not no slave or no master,need clear current mode.");
		gHAConfig.setSQLMode(gHAConfig.NotSlaveNotMaster);
	}
	
	DBHALOG_INFO( "HAFramework cleanup." );

	//TODO:开定时器和开任务线程应该弄到一个线程里面去做就可以了
	stopDBMonitor();
	destroyMysqlRecoverThread();
	stopDBMonitor();

	// 对于主机来说，不需要修改init状态
	if (gHAConfig.getSlaveConnectState() != SlaveConnectState[connectInit])
		gHAConfig.setSlaveConnectState(SlaveConnectState[connectFail]);
	return true;
}


bool CDBHAServer::getState( HAState& state)
{
	if(!m_is_master){
		state.state = HAState::isSlave;
	}else if(m_slave_is_online){
		state.state = HAState::activeStandby;
	}else if(!m_slave_is_online){
		state.state = HAState::singleActive;
	}else{
		DBHALOG_INFO("invalid state.");
		return false;
	}

	state.clusterId = gHAConfig.getClusterId();
	state.clusterUid = gHAConfig.getUId();

	return true;
}

bool CDBHAServer::stop()
{
	cleanup();
	gHAConfig.setSQLWriteState(gHAConfig.NotReadNotWrite);
	return true;
}

bool CDBHAServer::reloadDBHAConfig()
{
	cleanup();

	setWorkspace(DBHAFrameworkDir);

	g_DBHAConfig->reload();

	HAConfig cfg;
	getDBHAConfig(cfg);

	setHAConfig(cfg);

	startup();

	DBHALOG_INFO("reload DBHAConfig success.");
	return true;
}

void CDBHAServer::printVersion()
{
	/*todo*/
}

void CDBHAServer::processHALinkStatus( CHeartbeatMonitor::LinkState& state)
{
	//心跳ip在低位
	CHeartbeatMonitor::LinkState::iterator it = state.find( g_DBHAConfig->getLocalHeartbeatIP() );
	if ( it != state.end() && it->second == CHeartbeatMonitor::online ) {
		this->m_link_status |= 0x01;
	}else{
		this->m_link_status &= 0x10;
	}

	//业务ip在高位
	it = state.find( g_DBHAConfig->getLocalBusinessIP() );
	if ( it != state.end() && it->second == CHeartbeatMonitor::online ) {
		this->m_link_status |= 0x10;
	}else{
		this->m_link_status &= 0x01;
	}
}

bool CDBHAServer::haStateChange( CHeartbeatMonitor::HAAction action, CHeartbeatMonitor::LinkState& linkState, bool flag )
{
	DBHALOG_INFO( "ha start is changed, action:%d, flag:%d.", action, flag );
	switch( action ) {
		case CHeartbeatMonitor::becomeMaster: { 
			// NOTE:会中断恢复过程
			destroyMysqlRecoverThread();

			if (!gHAConfig.getSlaveChangeMasterFlag())
				DBHALOG_ERROR("the mysql server become master forcely,maybe lost data.");
			
			stopDBMonitor();
			
			bool ret = mysqlBecomeMaster();
			if (!ret){
				DBHALOG_INFO("mysql become master failed.");
				return false;
			}
			startDBMonitor(CHAConfigs::Master);
			
			//启动主机成功将mysql状态置为主机
			//该状态给运维查询用
			gHAConfig.setSQLMode(gHAConfig.Master);
			//开启主备同步服务.
			if( !m_sync_server.startup() ) {
				DBHALOG_ERROR( "sync server start failed." );
				//assert( 0 );
				//当sync server启动失败，如38060端口被占用，成不了主,服务退出  redmine:#66797
				gExitSem.post();
				return false;
			}
			m_is_master = true;
			m_slave_is_online = false;
			CHeartbeatMonitor::LinkState::iterator it = linkState.begin();
			for( ;it!=linkState.end(); it++){
				if(it->second==CHeartbeatMonitor::online){
					m_slave_is_online = true;
					break;
				}
			}

			gHAConfig.setSlaveConnectState(SlaveConnectState[connectInit]);
			gHAConfig.setSlaveSyncState(SyncState[syncInit]);
			DBHALOG_INFO("the DBHAServer become master.");
			return true;
		}
		case CHeartbeatMonitor::becomeSlave: {
			processHALinkStatus(linkState);
			if (flag == true){
				createMysqlRecoverThread();//创建恢复线程
				m_is_master = false;
				m_slave_is_online = false;

				//清理备机上命名为master的备份文件
				cleanMasterBackupFiles();

				DBHALOG_INFO("the DBHAServer become slave with recovery.");
				return true;
			}
			if( !m_sync_server.cleanup() ) {
				DBHALOG_ERROR( "sync server stop failed." );
			}
			stopDBMonitor();
			/*调用mysql成为备机的脚本*/
			bool ret = mysqlBecomeSlave();
			if (ret){
				startDBMonitor(CHAConfigs::Slave);
				DBHALOG_INFO("mysql become slave success.");
				gHAConfig.setSQLMode(gHAConfig.Slave);
			}else{
				DBHALOG_WARN( "mysql become slave failed, try to recover" );
				createMysqlRecoverThread();//创建恢复线程
			}

			m_is_master = false;
			m_slave_is_online = false;
			DBHALOG_INFO("the DBHAServer become slave.");

			//清理备机上命名为master的备份文件
			cleanMasterBackupFiles();

			return true;
			}
		case CHeartbeatMonitor::slaveOnline:
			m_slave_is_online = true;
			processHALinkStatus(linkState);
			DBHALOG_INFO("slaveOnline");
			return true;
		case CHeartbeatMonitor::slaveLinkChange:
			m_slave_is_online = true;
			processHALinkStatus(linkState);
			DBHALOG_INFO("slaveLinkChange");
			return true;
		case CHeartbeatMonitor::slaveOffline:
			m_slave_is_online = false;
			//备机挂掉将mysql主备状态置为单机
			//gHAConfig.setDBMasterSlaveState(0);
			//gHAConfig.saveSqlState();
			DBHALOG_INFO("slaveOffline");
			return true;

		case CHeartbeatMonitor::masterLinkChange:
			processHALinkStatus(linkState);
			DBHALOG_INFO("masterLinkChange");
			return true;

		// errordeal出现在双主、双备、主无法维持主状态、选举超时但无法成为主等场景
		case CHeartbeatMonitor::errordeal: 
			DBHALOG_ERROR("error, need to exit.");
			// 如果有恢复，需要等待恢复后唤醒
			if (g_DBHAConfig->getSlaveIsRecoveringFlag() == recovery){
				DBHALOG_WARN("waiting for recovery.");
				m_ha_waiting_for_recovery = new Infra::CSemaphore;
				m_ha_waiting_for_recovery->pend();
				delete m_ha_waiting_for_recovery;
				m_ha_waiting_for_recovery = NULL;
				DBHALOG_WARN("recovery is exit.");
			}

			gExitSem.post();
			return true;

		default:
			DBHALOG_ERROR( "invalid ha state[%d].", action );
			return true;
	}
}

//截取IP的后三位形成MySQL的serverid,如219.220.101.101则server id=220101101
bool CDBHAServer::getDBServerID(std::string localIP, std::string& serverID)
{
	DBHALOG_INFO( "the DBHA local IP :%s",localIP.c_str() );
	serverID="";
	StringVec itemvec;
	CommonFun::strSplit( localIP, '.', itemvec );
	int i=0;
	// ip只有四段,从ip第二段开始作为serverid
	for (StringVec::iterator it=itemvec.begin(); it != itemvec.end(); it++){
		if (i<1)
			i++;
		else
			serverID += *it;
	}

	DBHALOG_INFO("the mysql serverid %s.",serverID.c_str());
	return true;
}

bool CDBHAServer::mysqlBecomeMaster()
{
	char para[10240] = {0};
	if( g_DBHAConfig->getSingleMode() ){
		snprintf(para, 10240, "%s %s %s %s", "single", g_DBHAConfig->getMasterBackupPath().c_str(),
				gHAConfig.getUId().c_str(), m_server_id.c_str());
	}else{
		snprintf(para, 10240, "%s %s %s %s %s %s", "cluster", g_DBHAConfig->getMasterBackupPath().c_str(),
		gHAConfig.getUId().c_str(), m_server_id.c_str(), g_DBHAConfig->getPeerBusinessIP().c_str() ,
		g_DBHAConfig->getPeerHeartbeatIP().c_str());
	}

	bool ret = CommonFun::executeScript(g_DBHAConfig->getMasterScript(), para);
	if (!ret) {
		DBHALOG_ERROR( "mysqlBecomeMaster fail." );
		return false;
	}
	//stopDBMonitor();
	return true;
}

bool CDBHAServer::mysqlSlaveShellCmd( string binlogName, int32_t binlogPos)
{
	//优先使用心跳线，再使用业务线；心跳和业务都不通，则只能选心跳线
	bool useHeartBeatLine = false;
	if( m_link_status & 0x01 ){
		m_mysql_sync_ip = g_DBHAConfig->getPeerHeartbeatIP();
		useHeartBeatLine = true;
	}
	if( (m_link_status & 0x10) && (useHeartBeatLine == false) ){
		m_mysql_sync_ip = g_DBHAConfig->getPeerBusinessIP();
	}else{
		m_mysql_sync_ip = g_DBHAConfig->getPeerHeartbeatIP();
		useHeartBeatLine = true;
	}

	bool ret = mysqlSlaveShellCmd(m_mysql_sync_ip, binlogName, binlogPos);
	if( ret ){
		DBHALOG_INFO( "mysqlBecomeSlave success,host %s", m_mysql_sync_ip.c_str());
		return true;
	}

	DBHALOG_ERROR( "mysqlBecomeSlave failed,host %s", m_mysql_sync_ip.c_str());
	return false;
}

//通过脚本设置MySQL为slave
bool CDBHAServer::mysqlSlaveShellCmd( string host, string binlogName, int32_t binlogPos)
{
	char para[10240] = {0};
	snprintf(para, 10240, "%s %s %s %d %s %s", "startSlave", host.c_str() ,
		binlogName.c_str(),binlogPos, m_server_id.c_str(),g_DBHAConfig->getMasterBackupPath().c_str());

	bool ret = CommonFun::executeScript(g_DBHAConfig->getSlaveScript(), string(para));
	if (!ret) {
		DBHALOG_ERROR( "mysqlBecomeSlave fail,para %s", para );
		return false;
	}
	return true;
}

//TODO:备机设置binlog位置changeMasterFlag已经废弃不用了，但这里还没有更新，逻辑混乱,设置数据库为slave
bool CDBHAServer::mysqlBecomeSlave()
{
	bool ret = false;
	string master_file_name_tmp = gHeartBeatBinlogInfo.masterInitBinlogName;
	int32_t master_file_pos_tmp = gHeartBeatBinlogInfo.masterInitBinlogPos;

	if (gHAConfig.newMachine()){
		ret = mysqlSlaveShellCmd(master_file_name_tmp, master_file_pos_tmp);
	}else{
		if(gHeartBeatBinlogInfo.changeMasterFlag == -1){ // 默认值
		ret = mysqlSlaveShellCmd(master_file_name_tmp, master_file_pos_tmp);
		}else if(gHeartBeatBinlogInfo.changeMasterFlag == 0){ 
			// 强制,执行备机的信息，需要将主机信息导入，但是结果不看，直接恢复
			ret=mysqlSlaveShellCmd(master_file_name_tmp, master_file_pos_tmp);
		}else if (gHeartBeatBinlogInfo.changeMasterFlag == 1){ // 正常
			if (gHeartBeatBinlogInfo.changeMasterBinlogName != ""){
				master_file_name_tmp = gHeartBeatBinlogInfo.changeMasterBinlogName;
				master_file_pos_tmp = gHeartBeatBinlogInfo.changeMasterBinlogPos;
			}
			ret = mysqlSlaveShellCmd(master_file_name_tmp, master_file_pos_tmp);
		}
	}

	//待补充进入错误恢复流程
	//等待错误恢复完善
	if (!ret){
		//进入错误恢复流程
		DBHALOG_ERROR("mysql can not become slave normal,need goto error recover.");
		return false;
	}

	return true;
}

// 调用此函数的外部已经有锁保护
bool CDBHAServer::changeSyncIP()
{
	// 如果心跳和业务是一样的时候，不需要进行链路切换
	if( g_DBHAConfig->getPeerHeartbeatIP() == g_DBHAConfig->getPeerBusinessIP() ){
		return false;
	}

	if( m_mysql_sync_ip.empty() ){
		// 因为mysqlSlaveShellCmd()中初始就使用心跳线同步，所以异常时切换为业务线
		m_mysql_sync_ip = g_DBHAConfig->getPeerBusinessIP();
	}else if( m_mysql_sync_ip == g_DBHAConfig->getPeerHeartbeatIP() ){
		m_mysql_sync_ip = g_DBHAConfig->getPeerBusinessIP();	
	}else{
		m_mysql_sync_ip = g_DBHAConfig->getPeerHeartbeatIP();
	}

	std::ostringstream para;
	para << " changeSlaveInfo " << m_mysql_sync_ip.c_str();
	bool ret = CommonFun::executeScript(g_DBHAConfig->getSlaveScript(), para.str() );
	if (!ret) {
		DBHALOG_ERROR("change sync ip(%s) failed.", m_mysql_sync_ip.c_str());
		return false;
	}

	return true;
}

//检查主机数据库运行状态
void CDBHAServer::checkMasterTimeProc( unsigned long )
{
	if( g_DBHAConfig->getSingleMode() ){
		return;
	}

	Infra::CGuard g(m_check_mutex);
	int ret = CommonFun::executeScript(g_DBHAConfig->getMasterStateScript());
	if( ret ){
		DBHALOG_ERROR("check master running status failed,reset master!");
		mysqlBecomeMaster();
		return;
	}

	//DBHALOG_INFO("check master running status success,%d!", ret);
	return;
}

//检查备机数据库同步状态
void CDBHAServer::checkSlaveTimeProc( unsigned long )
{
	// 如果正在恢复中，检查同步必然失败，所以不检查
	if(g_DBHAConfig->getSlaveIsRecoveringFlag()){
		DBHALOG_INFO("slave is recoverying.");
		return;
	}
	
	Infra::CGuard g(m_check_mutex);
	string retResult;
	bool ret = CommonFun::getExecuteScriptValue(g_DBHAConfig->getSlaveStateScript(), "", retResult);
	if (ret){
		m_check_error_num = 0;
		gHAConfig.setSlaveSyncState(SyncState[syncing]);
		return;
	}
	
	DBHALOG_ERROR("slave sync error");
	gHAConfig.setSlaveSyncState(SyncState[syncError]);
	// 如果启动恢复线程成功，则会在startDBMonitor()中重置为0
	// 如果启动恢复线程失败，则下一次直接尝试去启动恢复线程，无需再反复重试多次切换IP
	m_check_error_num++; 
	
	if( m_check_error_num < slaveNotSyncNum ){
		DBHALOG_INFO("error count(%d) is less than %d, try to change sync ip.", m_check_error_num, slaveNotSyncNum);
		// 同步失败且次数未超过阈值，则尝试切换ip
		(void)changeSyncIP();
		return;
	}

	// 假设触发恢复后能正常，先重置错误次数
	m_check_error_num = 0;
	
	// NOTE:destroyMysqlRecoverThread()会中断恢复过程
	destroyMysqlRecoverThread();
	createMysqlRecoverThread();
}

//启动数据库监控定时器
void CDBHAServer::startDBMonitor(CHAConfigs::SQLMode mode)
{
	if (m_check_db_timer.isStarted()){
		DBHALOG_INFO( "m_check_db_timer is already started." );
		return;
	}

	m_check_error_num=0;
	bool ret = false;
	if( mode == CHAConfigs::Slave){
		ret = m_check_db_timer.start( CTimer::Proc( &CDBHAServer::checkSlaveTimeProc, this ),
			0, g_DBHAConfig->getCheckMysqlServicePeriod() );
	}else{
		ret = m_check_db_timer.start( CTimer::Proc( &CDBHAServer::checkMasterTimeProc, this ),
					0, g_DBHAConfig->getCheckMysqlServicePeriod() );
	}

	if ( !ret )
		DBHALOG_ERROR( "start m_check_db_timer fail,mode %d", mode );
	else
		DBHALOG_INFO( "start m_check_db_timer success,mode %d.", mode );
}

//停止数据库监控定时器
void CDBHAServer::stopDBMonitor()
{
	if (!m_check_db_timer.isStarted())
		return;

	if (!m_check_db_timer.stopAndWait())
		DBHALOG_ERROR( "stop m_check_mysql_service_timer fail" );
	else
		DBHALOG_INFO( "stop m_check_mysql_service_timer success." );
}

//启动数据库备份脚本
bool CDBHAServer::slaveMysqlDump()
{
	//备机上次恢复失败，不需要重新备份
	if(!g_DBHAConfig->getSlaveLastRecoveryResultFlag()){
		DBHALOG_INFO( "slave last recovery failed! slave do not need to start backup" );
		return true;
	}

	char para[10240] = {0};
	std::string uid;
	if( gHAConfig.newMachine() ){
		uid = "0xDEADNEWMACHINEBACKUP";
	}else{
		uid =  gHAConfig.getUId();
	}
	snprintf(para, 10240, "start %s %s %s slave",g_DBHAConfig->getSlaveBackupPath().c_str(), uid.c_str() ,
			m_server_id.c_str());

	bool ret = CommonFun::executeScript(g_DBHAConfig->getBackupScript(), string(para));
	if (!ret) {
		DBHALOG_ERROR( "slave:%s, mysqlMasterDump fail,para:%s.",m_server_id.c_str(), para );
		return false;
	}
	return true;
}

//停止数据库备份脚本
bool CDBHAServer::stopSlaveDump()
{
	char para[10240] = {0};
	snprintf(para, 10240, "stop %s %s %s slave ",g_DBHAConfig->getSlaveBackupPath().c_str(), gHAConfig.getUId().c_str() ,
		m_server_id.c_str());

	bool ret = CommonFun::executeScript(g_DBHAConfig->getBackupScript(), string(para));
	if (!ret) {
		DBHALOG_ERROR( "slave:%s, mysqlMasterDump fail.",m_server_id.c_str() );
		return false;
	}
	return true;
}

//给定恢复文件全路径校验恢复文件的md5值
bool CDBHAServer::checkRecoverFileMd5sum(std::string dump_file_name, std::string& md5sum)
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

//失败时需要唤醒外部的全局信号量，true则不需要
bool CDBHAServer::slaveMysqlRecovery()
{
	DBHALOG_INFO("slave mysql recover begin.\n");
	std::string para = "start ";
	std::string dump_file_name = g_DBHAConfig->getSlaveBackupPath()+"/"+
		g_DBHAConfig->getSlaveRecoverDumpFileName();

	if( !Infra::CFile::access( dump_file_name.c_str(), Infra::CFile::accessExist ) ){
		DBHALOG_ERROR( "dump file slaveRecoverFile is not exist" );
		g_DBHAConfig->setSlaveIsRecoveringFlag(notRecovery);
		return false;
	}

	//获取recover文件md5值
	std::string file,md5sum;
	int64_t pos;
	g_DBHAConfig->getSlaveRecoveringBinlog(file, pos, md5sum);
	if( true != checkRecoverFileMd5sum(dump_file_name, md5sum) ){
		DBHALOG_ERROR( "checkRecoverFileMd5sum slaveRecoverFile failed,md5:%s.\n", md5sum.c_str());

		//删除recover文件
		if( Infra::CFile::access( dump_file_name.c_str(), Infra::CFile::accessExist ) ){
			if( !Infra::CFile::remove( dump_file_name.c_str() ) ){
				DBHALOG_ERROR( "remove file slaveRecoverFile failed" );
			}
		}

		//通知主机重新生成备份文件
		//recreateMasterBackupFile();

		g_DBHAConfig->setSlaveIsRecoveringFlag(notRecovery);
		return false;
	}
	para+=dump_file_name;

	//设置DBHA正在recover,即使重启下次起来仍然做recover
	//配置文件中至上正在恢复的标记，防止恢复过程中出现掉电等异常
	g_DBHAConfig->setSlaveIsRecoveringFlag(recovery);

	// 在恢复之前先设置一次同步位置，由mysqldump内去修正binlog的位置;
    if( !mysqlSlaveShellCmd( file, pos ) ){
		DBHALOG_WARN( "start slave failed, master binlog is %s, position is %"PRIi64".", file.c_str(), pos );
	}
	
	//执行恢复之前杀掉已经存在的恢复进程及其子进程
	stopSlaveMysqlRecovery();

	//执行恢复
	bool ret = CommonFun::executeScript(g_DBHAConfig->getRecoveryScript(), para );

	// 如果在恢复中服务退出，则不需要判定恢复结果
	if (g_exit){
		DBHALOG_INFO( "return with exit.");
		return false;
	}

	//删除主机传过来的dump文件
	//放在recover脚本删除，防止强制杀恢复脚本，导致误删dump文件
/*
	if( Infra::CFile::access( dump_file_name.c_str(), Infra::CFile::accessExist ) ){
		if( !Infra::CFile::remove( dump_file_name.c_str() ) ){
			DBHALOG_ERROR( "remove file[%s] failed", dump_file_name.c_str() );
		}
	}*/

	if ( !ret ){
		//DBHALOG_INFO( "return with exit.");
		DBHALOG_ERROR( "slave :%s,recover fail.need exit DBHAServer.",m_server_id.c_str() );
		//故障恢复之后，清除之前设置的同步位置
		cleanSlaveRecoveryPos();
		return false;
	}

	/* 清除信息 */
	g_DBHAConfig->setSlaveIsRecoveringFlag(notRecovery);
	g_DBHAConfig->setSlaveRecoveringBinlog("", 0, "");
	if( g_DBHAConfig->serviceInExpandMode() ){
		g_DBHAConfig->serviceLeavingExpandMode();
		DBHALOG_INFO( "service leave expand mode now...\n" );
	}

	if (m_ha_waiting_for_recovery)
		m_ha_waiting_for_recovery->post();
	//if (!ret) {
		//DBHALOG_ERROR( "slave :%s,recover fail.need exit DBHAServer.",m_server_id.c_str() );
		//return false;
	//}

	DBHALOG_INFO("slave mysql recover end.\n");
	return true;
}

//创建恢复线程
void CDBHAServer::createMysqlRecoverThread()
{
	CGuard g(m_recovery_mutex);
	if (m_thread){
		DBHALOG_INFO("recovery thread is already exist.");
		return;
	}

	gHAConfig.setSlaveSyncState(SyncState[recoverying]);

	CThreadLite::ThreadProc proc(&CDBHAServer::recoverThread, this);
	m_thread = new CThreadLite(proc, "recoverThread");
	m_thread->createThread();
	DBHALOG_INFO("start slave mysql recover thread.");

}

//销毁恢复线程
void CDBHAServer::destroyMysqlRecoverThread()
{
	DBHALOG_INFO("enter to destory mysql recovery thread.");
	//尝试关闭dump线程
	stopSlaveDump();

	bool mysql_is_recovery=false;
	if(g_DBHAConfig->getSlaveIsRecoveringFlag())//如果当前备机正在恢复强制杀掉
	{
		DBHALOG_WARN("stop mysql recover.");
		stopSlaveMysqlRecovery();
		DBHALOG_WARN("stop mysql recover success.");
		mysql_is_recovery = true;//强制杀掉recover脚本需要将recovery标记致回去
	}

	CGuard g(m_recovery_mutex);
	if (m_thread){
		DBHALOG_INFO("enter to destory sync thread.");
		 // CThread::sleep(100);
		//m_thread->cancelThread();
		m_thread->destroyThread();
		DBHALOG_INFO("destory sync thread success.");
		delete m_thread;
		m_thread = NULL;
	}
	if(mysql_is_recovery)
	{
		g_DBHAConfig->setSlaveIsRecoveringFlag(recovery);
	}
}

// -1表示进程退出，0表示执行失败，1表示执行成功
bool CDBHAServer::getDumpFile()
{
	//slave端先本地dump一个文件
	//备机开启syncClient进行数据传输
	bool ret = slaveMysqlDump();
	// 如果中途退出了，则直接退出,不需要唤醒信号量
	if (g_exit){
		DBHALOG_INFO("slave:%s is exit.",m_server_id.c_str() );
		return false;
	}
	// 失败则直接退出整体程序
	if (!ret){
		DBHALOG_ERROR("slave:%s dump file failed,can not recover.",m_server_id.c_str());
		return false;
	}
	DBHALOG_INFO("slave:%s dump file success.",m_server_id.c_str() );

	CSyncClient sync_client;
	if (!sync_client.startup()){//失败有可能是syncClient已经启动
		DBHALOG_ERROR("start recover sync client fail.");
		return false;
	}

	//TODO
	sync_client.waitForDumping();
	if (sync_client.getSyncState() != sync_client.synced){
		DBHALOG_ERROR("slave:%s ,sync file failed, state:%d.",m_server_id.c_str(), (int)sync_client.getSyncState());
		sync_client.cleanup();
		return false;
	}
	// 此处有可能导致主机收不到end包，因为关闭太快了，但是不影响恢复
	std::string file,md5sum;
	int64_t pos;
	sync_client.getRecoverBinlogInfo(file, pos, md5sum);
	g_DBHAConfig->setSlaveRecoveringBinlog(file, pos, md5sum);

	if( !sync_client.cleanup() ) {
		DBHALOG_ERROR( "sync client stop failed." );
	}
	DBHALOG_INFO("slave:%s get master dump file success.",m_server_id.c_str() );
	return true;

}

//恢复线程执行体
void CDBHAServer::recoverThread(Infra::CThreadLite& thread)
{
	DBHALOG_INFO("slave:%s begin recovery.",m_server_id.c_str());
	// 先尝试关闭备机监控定时任务
	stopDBMonitor();

	//CGuard g(m_recovery_mutex);
	DBHALOG_INFO("slave need backup flag:%d",g_DBHAConfig->getSlaveLastRecoveryResultFlag());

	if (!getDumpFile()){
		if( g_DBHAConfig->getSlaveIsRecoveringFlag() && !g_exit ){
			DBHALOG_ERROR("slave recover failed, flag %d", g_DBHAConfig->getSlaveIsRecoveringFlag());
			gExitSem.post();
		}else{
			//获取文件失败，拉起监控任务再次执行监控
			startDBMonitor(CHAConfigs::Slave);
		}
		return;
	}

	//备机恢复接口
	bool ret = slaveMysqlRecovery();
	if( ret ){
		g_DBHAConfig->setSlaveLastRecoveryResultFlag(1);
		DBHALOG_INFO("Recover Success.slave need backup flag:%d",g_DBHAConfig->getSlaveLastRecoveryResultFlag());
		// 恢复成功就认为是已同步的
		gHAConfig.setSlaveSyncState(SyncState[syncing]);
		if( g_exit ){
			DBHALOG_INFO("server is exit.");
			return;
		}

		DBHALOG_INFO("slave:%s ,recover success.",m_server_id.c_str());
		gHAConfig.setSQLMode(gHAConfig.Slave);
		startDBMonitor(CHAConfigs::Slave);

		return;
	}else{
		g_DBHAConfig->setSlaveLastRecoveryResultFlag(0);
		DBHALOG_INFO("Recover Failed.slave need backup flag:%d",g_DBHAConfig->getSlaveLastRecoveryResultFlag());
		DBHALOG_ERROR("slave:%s ,recover failed, server will exit.",m_server_id.c_str());
		// 恢复失败则设置为syncError，如果维持recoverying状态，则会导致重启后masterJudger()判断时一直无法成为主
		gHAConfig.setSlaveSyncState(SyncState[syncError]);
		// 恢复失败时，总是退出
		// g_exit为true说明已经调用了gExitSem.post()，此处不能再调用，否则gExitSem析构时断言会失败
		if( !g_exit ){
			gExitSem.post();
		}
	}

	return;
}

//停止恢复线程
void CDBHAServer::stopSlaveMysqlRecovery()
{
	std::string para = "stop ";
	// TODO建议此处所有的dump绝对路径在dbhaconfig中，统一函数
	std::string dump_file_name = g_DBHAConfig->getSlaveBackupPath()+"/"+
		g_DBHAConfig->getSlaveRecoverDumpFileName();
	para+=dump_file_name;
	bool ret = CommonFun::executeScript(g_DBHAConfig->getRecoveryScript(),para );
	if (!ret) {
		DBHALOG_ERROR( "stop slave :%s,recover fail.",m_server_id.c_str() );
		return;
	}
	DBHALOG_INFO( "stop slave :%s,recover success.",m_server_id.c_str() );
	return;
}

//清理备机同步位置
void CDBHAServer::cleanSlaveRecoveryPos()
{
	std::string para = "cleanSlave ";

	bool ret = CommonFun::executeScript(g_DBHAConfig->getSlaveScript(),para );
	if (!ret) {
		DBHALOG_ERROR( "clean slave recovery pos:%s,fail.",m_server_id.c_str() );
		return;
	}
	DBHALOG_INFO( "clean slave recovery pos :%s,success.",m_server_id.c_str() );
	return;
}

//成为备机后，删除掉备机上的命名为master的备份文件
void CDBHAServer::cleanMasterBackupFiles()
{
	std::string para = "cleanMasterBackup ";

	bool ret = CommonFun::executeScript(g_DBHAConfig->getSlaveScript(),para );
	if (!ret) {
		DBHALOG_ERROR( "clear master backup files in slave fail.");
		return;
	}
	DBHALOG_INFO( "clear master backup files in slave success.");
	return;
}

//恢复文件md5校验失败后，通知主机重新生成备份文件
void CDBHAServer::recreateMasterBackupFile()
{
	std::string para = "createMasterBackupFile ";

	bool ret = CommonFun::executeScript(g_DBHAConfig->getSlaveScript(),para );
	if (!ret) {
		DBHALOG_ERROR( "create master backup files fail.");
		return;
	}
	DBHALOG_INFO( "create master backup files success.");
	return;
}


bool CDBHAServer::masterJudger( std::stringstream& judgement )
{
	// mysql服务正常，才能成为主
	int mode = checkMysqlMode();
	if( 0 > mode ){
		judgement << "[FAIL] local mysql is abnormal.\n";
		return false;
	}
	judgement << "[PASS] local mysql is normal.\n";
	
	// mysql已经是主，则无需判断后续条件
	if( 1 == mode ){
		judgement << "[PASS] local mysql is already master.\n";
		return true;
	}
	
	// 单机模式,可以直接成主
	if( g_DBHAConfig->getSingleMode() ){
		judgement << "[PASS] single mode.\n";
		return true;
	}

	// 扩容模式且历史状态是主，可以直接成主
	if( g_DBHAConfig->serviceInExpandMode() ){
		if( CHeartbeatMonitor::master == gHAConfig.getUsedState() ){
			judgement << "[PASS] expand mode and history state is master\n";
			return true;
		}else{
			judgement << "[FAIL] expand mode and history state is not master\n";
			return false;
		}
	}

	// 处于恢复过程中时，不允许升主
	bool recovering = false;
	if( m_check_mutex.tryEnter() ){
		std::string sync_state = gHAConfig.getSlaveSyncState();
		// redmine #29826
		// FIXME: 2018.9.25, 此处的实现依赖于部分特定的实现，不合理也不易理解，详见问题单
		if( ( sync_state == SyncState[recoverying] )
			|| ( sync_state == SyncState[syncError] && ( slaveNotSyncNum - m_check_error_num < 3 ) ) ){			
			recovering = true;
		}
		m_check_mutex.leave();
	}else{
		// 未拿到锁，认为checkSlaveTimeProc()本次可能触发恢复
		recovering = true;
	}
	
	if( recovering ){
		judgement << "[FAIL] local mysql is recovering\n";
		return false;
	}
	judgement << "[PASS] local mysql is not recovering\n";
	
	// 检查主机的mysql是否可用
	if( isMysqlMasterUseable() ){
		judgement << "[FAIL] mysql master is still useable.\n";
		return false;
	}
	judgement << "[PASS] mysql master is not useable.\n";
	
	// 拥有最新数据才能成为主（允许存在一定偏差）
	if( !hasLastestData( judgement ) ){
		return false;
	}
	
	return true;
}

bool CDBHAServer::fastMasterJudger( std::stringstream& judgement )
{
	// 以下条件的判断结果已经在masterJudger()中记录，此处不再记录
	(void)judgement;
	
	// 以下两种情况，允许快速成主
	// 1.单机模式
	// 2.扩容模式且历史状态是主(历史状态为主，说明其数据是最新的)
	if( g_DBHAConfig->getSingleMode() 
		|| ( g_DBHAConfig->serviceInExpandMode() && CHeartbeatMonitor::master == gHAConfig.getUsedState() ) ){
		return true;
	}

	return false;
}

int CDBHAServer::checkMysqlMode()
{
	std::string mode;
	(void)CommonFun::getExecuteScriptValue(g_DBHAConfig->getMysqlServerScript(), "mode", mode);
	if( mode.empty() ) {
		DBHALOG_ERROR( "get mysql mode failed." );
		return -1;
	}

	if( 0 == mode.compare( 0, 6, "master" ) ){
		return 1;
	}

	return 0;
}

bool CDBHAServer::isMysqlMasterUseable()
{
	// 检查远端的主Mysql是否可用
	// 可用的标准：1.虚IP能通。2.mysql服务正常
	std::string vip = g_DBHAConfig->getVIP();
	string binlog_info;
	bool ret = CommonFun::getExecuteScriptValue( g_DBHAConfig->getBinlogInfoScript(), vip, binlog_info);
	if( ret ){
		// 进入该函数说明已经有段时间没收到对端心跳了，使用从主mysql获取的binlog信息
		CommonFun::setBinlogInfo(binlog_info, 
								 gHeartBeatBinlogInfo.masterCurrentBinlogName, 
								 gHeartBeatBinlogInfo.masterCurrentBinlogPos );
		DBHALOG_INFO( "update master binlog from master mysql, vip:%s", vip.c_str() );
	}else{
		DBHALOG_ERROR( "get master state failed, vip:%s", vip.c_str() );
	}

	return ret;
}

bool CDBHAServer::hasLastestData( std::stringstream& judgement )
{
	// 代码流程上保证只有mysql是备时，才会进入该函数
	if( gHeartBeatBinlogInfo.masterCurrentBinlogName.empty() ){
		// 没有收到过对端信息，无法确定本端是否拥有最新数据
		judgement << "[FAIL] the name of master binlog is empty.\n";
		return false;
	}
	
	// 获取本机的同步信息
	std::string slaveInfo;
	bool ret = CommonFun::getExecuteScriptValue(g_DBHAConfig->getCheckSyncState(), "", slaveInfo);
	if( slaveInfo.empty() ) {
		judgement << "[FAIL] get local binlog information failed.\n";
		return false;
	}

	std::string binlogName;
	int32_t binlogPos;
	ret = CommonFun::setBinlogInfo(slaveInfo, binlogName, binlogPos);
	if( !ret ) {
		judgement << "[FAIL] the local binlog information(" << slaveInfo << ") is invalid.\n";
		return false;
	}
	
	// 获取对端的binlog的index
	int masterIndex = 0;
	int retS = sscanf(gHeartBeatBinlogInfo.masterCurrentBinlogName.c_str(), "%*[^.].%d", &masterIndex);
	if( retS != 1 ){
		judgement << "[FAIL] the name of master binlog(" << gHeartBeatBinlogInfo.masterCurrentBinlogName << ") is invalid.\n";
		return false;
	}
	
	// 获取本地的binlog的index
	int slaveIndex = 0;
	if( sscanf(binlogName.c_str(), "%*[^.].%d", &slaveIndex) != 1 ){
		judgement << "[FAIL] the name of local binlog(" << binlogName << ") is invalid.\n";
		return false;
	}

	// 偏差在允许范围内，可认为拥有最新数据
	int index_diff = masterIndex - slaveIndex;
	if(index_diff > g_DBHAConfig->getBinlogFileDelayNum() ){
		judgement << "[FAIL] the difference of binlog index(" << index_diff << ") is greater than " 
			      << g_DBHAConfig->getBinlogFileDelayNum() << ".\n";
		return false;
	}
	judgement << "[PASS] the difference of binlog index(" << index_diff << ") is less than " 
			  << g_DBHAConfig->getBinlogFileDelayNum() << ".\n";
	
	if( masterIndex == slaveIndex ){
		// 比较同一个binlog，两端的差值
		int32_t diff = gHeartBeatBinlogInfo.masterCurrentBinlogPos - binlogPos;
		int32_t high_threshold = g_DBHAConfig->getDiffPosHighThreshold() << 20;
		int32_t low_threshold = g_DBHAConfig->getDiffPosLowThreshold() << 20;
		if( high_threshold < diff ){
			judgement << "[FAIL] the difference of same binlog position (" << diff << ") is greater than " 
				      << high_threshold << ".\n";
			return false;
		}
		
		if( low_threshold < diff && diff <= high_threshold ){
			uint64_t now = Infra::CTime::getCurrentMilliSecond() / 1000;
			if( 0 == m_last_check_binlog_time ){
				m_last_check_binlog_time = now;
			}
			if( ( now - m_last_check_binlog_time ) < (uint64_t)g_DBHAConfig->getWaitToBecomeMasterTime() ){
				judgement << "[FAIL] the difference of same binlog position (" << diff << ") is between in "
					      << low_threshold << " and " << high_threshold << ", wait for a moment.\n";
				return false;
			}
			judgement << "[PASS] the difference of same binlog position (" << diff << ") is between in "
					  << low_threshold << " and " << high_threshold << ", but has been wait for "
					  << g_DBHAConfig->getWaitToBecomeMasterTime() << " seconds.";
			m_last_check_binlog_time = 0;
			return true;
		}
		
		judgement << "[PASS] the difference of same binlog position("<< diff<<") is less than " 
		          << low_threshold << ".\n";

	}
	// 每当返回true时，下次重新计算时间
	m_last_check_binlog_time = 0;
	return true;
}

}//DBHA
}//VideoCloud
}//Dahua

