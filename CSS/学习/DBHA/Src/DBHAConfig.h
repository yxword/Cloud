//
//  "$Id$"
//
//  Copyright ( c )1992-2016, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	数据库HA框架配置文件项操作
//	Revisions:		2016-12-2 #
//
#ifndef __DAHUA_VIDEO_CLOUD_DB_HA_CONFIG_H__
#define __DAHUA_VIDEO_CLOUD_DB_HA_CONFIG_H__

#include <string>
#include "Common/ConfigTool.h"
#include "Infra/Types.h"
#include "Infra/Time.h"

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

typedef struct HAConfig
{
	std::string multicastIP;       //组播IP,DBHA使用224.0.8.5
	uint32_t multicastPort;        //组播端口,DBHA使用5000
	std::string localHeartbeatIP;  //本机心跳IP
	std::string peerHeartbeatIP;   //对端心跳IP
	std::string localBusinessIP;   //本机业务IP
	std::string peerBusinessIP;    //对端业务IP
	uint32_t syncPort;             //故障恢复时主备通信端口,默认38060
	uint32_t clusterPort;          //暂未使用
	std::string vip;               //DBHA虚IP
	std::string vipNetmask;        //虚IP掩码
	std::string vifName;           //虚IP对应网卡
	std::string ifName;            //业务IP实网卡
	std::string heartbeatifName;   //心跳IP实网卡
	std::string serviceName;       //服务名,DBHAService
	std::string userDefined;       //用户自定义数据
}HAConfig;

typedef struct MasterBinlogInfo
{
	std::string masterInitBinlogName;		//主机新安装时的binlog名字信息
	int32_t masterInitBinlogPos;			//主机新安装时的binlog位置信息
	std::string changeMasterBinlogName;		//成为主机时的binlog名字
	int32_t changeMasterBinlogPos;			//成为主机时的binlog位置
	int32_t changeMasterFlag;				//备机在成为主机时是否合法
	std::string masterCurrentBinlogName;	//主机当前binlog名字
	int32_t masterCurrentBinlogPos;			//主机当前binlog位置
	uint64_t masterCurrentTime;				//主机获取当前binlog信息的时间

	MasterBinlogInfo()
	{
		masterInitBinlogName="";
		masterInitBinlogPos = 0;
		changeMasterBinlogName="";
		changeMasterBinlogPos = 0;
		changeMasterFlag = -1;
		masterCurrentBinlogName="";
		masterCurrentBinlogPos=0;
		masterCurrentTime = Infra::CTime::getCurrentMilliSecond();
	}

}MasterBinlogInfo;

const std::string managerDomain = "DBHA";

// 本地心跳ip.
const std::string managerLocalHeartbeatIP = "local_heartbeat_IP";
const std::string managerLocalHeartbeatIPDefault = "";

// 对端心跳ip.
const std::string managerPeerHeartbeatIP = "peer_heartbeat_IP";
const std::string managerPeerHeartbeatIPDefault = "";

// 本地业务ip.
const std::string managerLocalBusinessIP = "local_business_IP";
const std::string managerLocalBusinessIPDefault = "";

// 对端业务ip.
const std::string managerPeerBusinessIP = "peer_business_IP";
const std::string managerPeerBusinessIPDefault = "";

// 集群虚ip.
const std::string managerVIP = "DBHA_virtual_IP";
const std::string managerVIPDefault = "";

// 集群网关
const std::string managerGateway = "DBHA_gateway";
const std::string managerGatwayDefault = "";

// 集群子网掩码.
const std::string managerVIPNetmask = "DBHA_virtual_IP_netmask";
const std::string managerVIPNetmaskDefault = "";

// 主备端口号.
const std::string managerSyncPort = "DBHA_sync_port";
const int32_t managerSyncPortDefault = 38060;


// 线程数.
const std::string managerThreadNum = "DBHA_thread_num";
const int32_t managerThreadNumDefault = 10;


// 组播ip.
const std::string managerMulticastIP = "DBHA_multicast_IP";
const std::string managerMulticastIPDefault = "224.0.6.5";

// 组播端口.
const std::string managerMulticastPort = "DBHA_multicast_port";
const int32_t managerMulticastPortDefault = 5000;
// 虚网卡名, 比如bond0:0.
const std::string managerVifName = "DBHA_vif_name";
const std::string managerVifNameDefault = "bond2:8";

// 真实网卡名, 比如bond0, 用于arping.
const std::string managerIfName = "DBHA_if_name";
const std::string managerIfNameDefault = "bond2";

const std::string managerHeartbeatIfName = "DBHA_heartbeart_if_name";
const std::string managerHeartbeatIfNameDefault = "bond3";

// Media-manager日志等级
const std::string DBHALogLevel = "DBHA_log_level";
const int32_t DBHALogLevelDefault = 4; //info级别. 0:不输出, 1:fatal, 2: error, 3:warn, 4:info, 5:Trace, 6 debug.


// 服务名称, 用于区分不同集群.
const std::string managerServiceName = "DBHA_service_name";
const std::string managerServiceNameDefault = "DBHAService";

// 用户自定义字段, 集群匹配时会使用.
const std::string managerUserDefined = "DBHA_user_defined";
const std::string managerUserDefinedDefault = "";

//mysql服务检测时间周期
const std::string checkMysqlServicePeriod = "check_mysql_service_interval_ms";
const int32_t checkMysqlServicePeriodDefault = 3000;

//mysql服务连续多少次为stop状态，则自杀DBHA服务
const std::string maxMysqlServiceStopStateNum = "max_mysql_service_stop_state_num";
const int32_t maxMysqlServiceStopStateNumDefault = 3;

//mysql主备关系正常主机框架挂掉,备机等待主机多少个心跳周期才认为主机真实挂掉
const std::string maxSlaveSyncWithMasterNum = "max_slave_sync_with_master_num";
const int32_t maxSlaveSyncWithMasterNumDefault = 6;

//mysql主机挂掉,mysql备机成为主机之前发现主备同步有问题，等待多少个心跳周期才强制成为主
//const std::string maxMysqlSlaveWaitMasterUpNum = "max_mysql_slave_wait_master_num";
//const int32_t maxMysqlSlaveWaitMasterUpNumDefault = 6;

const std::string mysqlShellDir = "/cloud/dahua/DataBaseServer/DBHA/Shell/";
const std::string mysqlShellSetUpDir = "/cloud/dahua/DataBaseServer/DBHA/Shell/MysqlSetup/";
const std::string mysqlShellBinlogDir = "/cloud/dahua/DataBaseServer/DBHA/Shell/Binlog/";
const std::string mysqlShellBackupDir = "/cloud/dahua/DataBaseServer/DBHA/Shell/Backup/";
const std::string mysqlShellCommonDir = "/cloud/dahua/DataBaseServer/DBHA/Shell/Common/";
const std::string mysqlShellDeployDir = "/cloud/dahua/DataBaseServer/DBHA/Shell/Deploy/";
const std::string mysqlShellRecoveryDir = "/cloud/dahua/DataBaseServer/DBHA/Shell/Recovery/";
const std::string mysqlShellServiceDir = "/cloud/dahua/DataBaseServer/DBHA/Shell/Service/";

//mysql 设置server-id 脚本位置
const std::string mysqlSetServerIDScript = "mysql_server_id_script";
const std::string mysqlSetServerIDScriptDefault = mysqlShellSetUpDir + "Prepare.sh";

//change master 脚本位置
const std::string mysqlMasterScript = "mysql_master";
const std::string mysqlMasterScriptDefault = mysqlShellSetUpDir + "Master.sh";

//change slave 脚本位置
const std::string mysqlSlaveScript = "mysql_slave";
const std::string mysqlSlaveScriptDefault = mysqlShellSetUpDir + "Slave.sh";

//mysql binlog 获取的脚本位置
const std::string mysqlGetBinlogScript = "mysql_binlog_info";
const std::string mysqlGetBinlogScriptDefault = mysqlShellBinlogDir + "GetBinlogInfo.sh";

//slave 状态获取脚本位置
const std::string mysqlGetSlaveStateScript = "mysql_slave_state";
const std::string mysqlGetSlaveStateScriptDefault = mysqlShellServiceDir + "GetSlaveState.sh";

//查询主机MySQL运行状态
const std::string mysqlCheckMasterStateScript = "mysql_master_state";
const std::string mysqlCheckMasterStateScriptDefault = mysqlShellServiceDir + "CheckMasterState.sh";

//MySQL主机备份路径
const std::string mysqlMasterBackupPath = "mysql_backup_path";
const std::string mysqlMasterBackupPathDefault = "/cloud/dahua/DataBaseServer/mysql_backup";

//MySQL备机备份路径
const std::string mysqlSlaveBackupPath = "mysql_backup_path";
const std::string mysqlSlaveBackupPathDefault = "/cloud/dahua/DataBaseServer/mysql_backup";

// 备机本地备份自己的数据，用法：sh Backup.sh start 目录  uuid  mysql的server_id; sh Backup.sh stop 目录  uuid  mysql的server_id 
// 全路径的方式/data/uuid_serverID，脚本自动会往后加时间戳变成/data/uuid_serverID_年月日时分秒
const std::string mysqlBackupScript = "mysql_backup_script";
const std::string mysqlBackupScriptDefault = mysqlShellBackupDir  + "Backup.sh";

// 备机本地备份自己的数据,用法：sh Recovery.sh start filename(全路径); sh Recovery.sh stop filename(全路径)
const std::string mysqlRecoveryScript = "mysql_recovery_script";
const std::string mysqlRecoveryScriptDefault = mysqlShellRecoveryDir + "Recovery.sh";

// 获取主机备份文件,用法：sh OpMasterBackupFile.sh 目录  uuid  mysql的server_id
const std::string mysqlMasterBackupFile = "mysql_master_backup_file";
const std::string mysqlMasterBackupFileDefault = mysqlShellBackupDir + "OpMasterBackupFile.sh";

//校验同步状态
const std::string mysqlCheckSyncState = "mysql_get_slave_info";
const std::string mysqlCheckSyncStateDefault = mysqlShellServiceDir + "CheckSyncState.sh";

//备机恢复临时文件名
const std::string slaveRecoveryDumpFileName = "slave_recovery_dump_file_name";
const std::string slaveRecoveryDumpFileNameDefault="slaveRecoverFile";

//备机是否正在进行错误恢复,1表示正在恢复
const std::string slaveIsRecovering =  "slave_is_recovering";
const int32_t slaveIsRecoveringDefault = 0;

//备机上一次是否恢复是否执行成功
const std::string slaveLastRecoveryResult =  "last_recovery_success";
const int32_t slaveLastRecoveryResultDefault = 1;

//备机恢复文件对应的binlog文件名
const std::string slaveRecoverBinlogFile =  "slave_is_recovering_binlog_file";
const std::string slaveRecoverDefault = "";

//备机恢复文件对应的binlog pos点
const std::string slaveRecoverBinlogPos =  "slave_is_recovering_binlog_pos";
const int64_t slaveRecoverBinlogPosDefault = 0;

//备机恢复文件的md5值
const std::string slaveRecoverMd5sum =  "slave_is_recovering_md5sum";
const std::string slaveRecoverMd5sumDefault = "";
//备机检测完全同步，是否需要从主机获取信息进行比较，1表示需要从主机进行对比
const std::string checkSyncWithMaster =  "check_sync_with_master";
const int32_t checkSyncWithMasterDefault = 1;

const std::string dumpFileValidNum = "dump_file_valid_num";
const int32_t dumpFileValidNumDefault = 5;

const std::string binlogFileDelayNum = "DBHA_binlog_file_delay_num";
const int32_t binlogFileDelayNumDefault = 0;

//DBHA扩容标记
const std::string expandHAFlag = "DBHA_expand_flag";
const int32_t expandHAFlagDefault = -1;

const std::string mysqlServerScript = "mysql_server_script";
const std::string mysqlServerScriptDefault = mysqlShellServiceDir + "MysqlServer.sh";

const std::string sameBinlogDiffPosHighThreshold = "DBHA_same_binlog_diff_pos_high_threshold";
const int32_t sameBinlogDiffPosHighThresholdDefault = 50; // 单位MB

const std::string sameBinlogDiffPosLowThreshold = "DBHA_same_binlog_diff_pos_low_threshold";
const int32_t sameBinlogDiffPosLowThresholdDefault = 5; // 单位MB

// binlog的position差距在阈值范围内时，需要等待一段时间才能成主
const std::string waitToBecomeMaster = "DBHA_wait_to_become_master_time";
const int32_t waitToBecomeMasterDefault = 300; // 单位秒

class CDBHAConfig
{
public:

	~CDBHAConfig();

	static CDBHAConfig* instance( std::string file = "" );
	Dahua::EFS::CConfigTool* getConfigTool() {return m_configTool;}
	bool init( std::string& file );
	bool reload();

	std::string getLocalHeartbeatIP();
	bool setLocalHeartbeatIP( const std::string& ip );

	std::string getPeerHeartbeatIP();
	bool setPeerHeartbeatIP( const std::string& ip );

	std::string getLocalBusinessIP();
	bool setLocalBusinessIP( const std::string& ip );

	std::string getPeerBusinessIP();
	bool setPeerBusinessIP( const std::string& ip );

	std::string getVIP();
	bool setVIP( const std::string& ip );

	std::string getVIPNetmask();
	bool setVIPNetmask( const std::string& ip );

	int32_t getSyncPort();
	bool setSyncPort( const int32_t port );

	int32_t getThreadNum();
	bool setThreadNum( int32_t& num );

	std::string getMulticastIP();
	bool setMulticastIP( const std::string& ip );

	int32_t getMulticastPort();
	bool setMulticastPort( const int32_t port );

	std::string getVifName();
	bool setVifName( const std::string& vifName );

	std::string getIfName();
	bool setIfName( const std::string& ifName );

	std::string getHeartbeatIfName();

	std::string getServiceName();
	bool setServiceName( const std::string& name );

	std::string getUserDefined();
	bool setUserDefined( const std::string& user );

	// 获取DBHA 日志接口.
	int32_t getLogLevel();

	int32_t getCheckMysqlServicePeriod();

	int32_t getMaxServiceStopNum();

	int32_t getMaxSlaveSyncWithMasterNum();

	int32_t getMaxMysqlSlaveWaitMasterUpNum();

	std::string getSetServerIDScript();
	std::string getMasterScript();
	std::string getSlaveScript();
	std::string getBinlogInfoScript();
	std::string getSlaveStateScript();

	std::string getMasterBackupPath();
	std::string getSlaveBackupPath();

	std::string getBackupScript();
	std::string getRecoveryScript();
	std::string opMasterBackupFile();

	std::string getSlaveRecoverDumpFileName();
	
	int32_t getSlaveIsRecoveringFlag();
	bool setSlaveIsRecoveringFlag( const int32_t flag );

	int32_t getSlaveLastRecoveryResultFlag();
	bool setSlaveLastRecoveryResultFlag( const int32_t flag );

	bool getSingleMode();

	int32_t getCheckSyncFlag();

	int getDumpFileValidNum();

	std::string getCheckSyncState();
	int32_t getBinlogFileDelayNum();

	bool serviceInExpandMode();
	bool serviceLeavingExpandMode();

	bool getSlaveRecoveringBinlog( std::string &binlogFile, int64_t &binlogPos, std::string &md5sum );
	bool setSlaveRecoveringBinlog( std::string binlogFile, int64_t binlogPos, std::string md5sum );

	std::string getMasterStateScript();

	std::string getMysqlServerScript();

	int32_t getDiffPosHighThreshold();
	int32_t getDiffPosLowThreshold();

	int32_t getWaitToBecomeMasterTime();
private:
	CDBHAConfig();
	CDBHAConfig( std::string file );
	CDBHAConfig( const CDBHAConfig& );
	CDBHAConfig& operator=( const CDBHAConfig& );
	std::string display();
private:
	static CDBHAConfig* m_pSignle;
	std::string m_configFile;
	Dahua::EFS::CConfigTool* m_configTool;
};

#define g_DBHAConfig CDBHAConfig::instance()

}//DBHA
}//VideoCloud
}//Dahua

#endif // __DAHUA_VIDEO_CLOUD_DB_HA_CONFIG_H__
