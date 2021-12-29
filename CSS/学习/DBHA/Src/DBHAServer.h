//
//  "$Id$"
//
//  Copyright ( c )1992-2016, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	mysql数据库监控服务
//	Revisions:		2016-12-2 #
//

#ifndef __DAHUA_VIDEO_CLOUD_DB_HA_SERVER_H__
#define __DAHUA_VIDEO_CLOUD_DB_HA_SERVER_H__

#include "HAMonitor.h"
#include "HeartbeatMonitor.h"
#include "DBHAConfig.h"
#include "HAConfigs.h"
#include <string>
#include <vector>
#include "Infra/Timer.h"
#include "SyncServer.h"
#include "SyncClient.h"

namespace Dahua {
namespace VideoCloud {
namespace DBHA {


typedef struct HAState
{
	//获取主机状态.
	enum MasterState{
		activeStandby=0, //正常主备状态.
		singleActive=1, //独立主机状态.
		activeLearning=2, //主备同步状态.
		isSlave=3		//是备机.
	};

	MasterState state;
	std::string clusterUid;
	uint8_t clusterId;
}HAState;

class CDBHAServer
{

public:
	CDBHAServer();
	~CDBHAServer();
	//设置集群工作路径. 用于集群自身的配置文件和集群信息存储, 比如集群id, 上次状态, 成为master的时间
	bool setWorkspace(std::string& path);

	//从配置文件获取DBHA的配置信息
	bool getDBHAConfig(HAConfig& cfg);

	//设置集群配置信息
	bool setHAConfig(HAConfig& cfg);

	//获取集群ID的接口
	uint8_t getClusterID();

	//清空集群id信息.
	bool clearClusterID();

	//初始化
	bool init();

	//启动服务.
	bool startup();

	//反初始化.
	bool cleanup();

	//获取状态.
	bool getState( HAState& state );

	//停止服务
	bool stop();

	//重新加载DBHA配置文件如修改虚ip
	bool reloadDBHAConfig();

	bool mysqlBecomeMaster();

	bool mysqlBecomeSlave();

	//底层调用
	bool mysqlSlaveShellCmd( std::string binlogName = "",
		int32_t binlogPos = 0 );
	bool mysqlSlaveShellCmd( std::string host, std::string binlogName, int32_t binlogPos);

	//备机mysql服务恢复
	bool slaveMysqlRecovery();

	//创建和消除备机恢复线程
	void createMysqlRecoverThread();
	void destroyMysqlRecoverThread();

	bool checkRecoverFileMd5sum(std::string dump_file_name, std::string& md5sum);

protected:

	void printVersion();

	//底层HA状态回调
	bool haStateChange( CHeartbeatMonitor::HAAction action, CHeartbeatMonitor::LinkState& linkState, bool flag=false );

private:

	//获取mysql服务server id
	bool getDBServerID(std::string localIP, std::string& serverID);

	// 找出所有slave信息
	void findSlaveState(std::string data, int32_t& connectMaster, int32_t& ioThread, int32_t& sqlThread,
		std::string& masterFile, std::string& relayLog, int32_t& readPos, int32_t& execPos);
	
	// 切换同步的ip
	bool changeSyncIP();

	// 检测备机状态
	void checkSlaveTimeProc(unsigned long);
	void checkMasterTimeProc( unsigned long );
	void startDBMonitor(CHAConfigs::SQLMode mode);
	void stopDBMonitor();

	bool getDumpFile();
	//线程执行函数
	void recoverThread(Infra::CThreadLite& thread);

	//slave mysqldump file
	bool slaveMysqlDump();
	bool stopSlaveDump();

	//停止备机恢复,进程退出如果备机正在恢复需要强制停止恢复，否则销毁线程时间久
	void stopSlaveMysqlRecovery();

	//故障恢复失败之后，用于清理备机已设置的同步位置
	void cleanSlaveRecoveryPos();

	//成为备机后，删除掉备机上的命名为master的备份文件
	void cleanMasterBackupFiles();

	//恢复文件md5校验失败后，通知主机重新生成备份文件
	void recreateMasterBackupFile();

	bool masterJudger( std::stringstream& judgement );
	bool fastMasterJudger( std::stringstream& judgement );
	
	/**
	 * @brief 获取mysql的主从模式
	 * @retval 1 master
	 * @retval 0 slave
	 * @retval -1 获取失败
	 */
	int checkMysqlMode();
	bool isMysqlMasterUseable();
	bool hasLastestData( std::stringstream& judgement );
	void processHALinkStatus( CHeartbeatMonitor::LinkState& state);
	
private:

	enum slave_sync_result{
		syncInit=0, ///< 初始状态，用于master状态
		syncing,  ///< 正常同步中，用于slave状态
		syncError, ///< 同步失败，会触发恢复，用于slave状态
		recoverying, ///< 正在恢复中，用于slave状态
	};

	enum slave_connect_master_state{
		connectInit=0,
		connectSuccess,
		connectFail,
	};
	enum slave_io_state{
		ioInit=0,
		ioSuccess,
		ioError,
	};

	enum slave_sql_state{
		sqlInit=0,
		sqlSuccess,
		sqlError,
	};

	enum recovery_state{
		notRecovery=0,
		recovery=1,
	};

	CHAMonitor m_monitor;
	bool m_is_master;
	bool m_slave_is_online;

	Infra::CTimer m_check_db_timer; // 如果成为备机，则需要定期检测备机状态

	//int32_t m_slave_connect_master;
	// 同步状态信息
	//std::string m_slave_sync_flag;
	//int32_t m_slave_IO_thread;
	//int32_t m_slave_sql_thread;
	//int64_t m_slave_read_master_pos;
	//int64_t m_slave_exec_master_pos;
	//std::string m_slave_read_master_file;
	//std::string m_slave_exec_master_file;


	std::string m_server_id;

	CSyncServer m_sync_server;		//dump文件传送server端

	Infra::CThread*	m_thread;
	Infra::CMutex m_recovery_mutex;
	Infra::CSemaphore  *m_ha_waiting_for_recovery;	// ha在推出的时候，需要等待恢复完成

	Infra::CMutex m_check_mutex;	// 检测锁，防止在准备关闭的时候，第二次的定时器触发出来
	uint8_t m_check_error_num;

	std::string m_mysql_sync_ip;	// mysql同步ip
	uint64_t m_last_check_binlog_time; ///< 记录上一次检查binlog的事件

	// 链路状态
	volatile int16_t m_link_status;
};

}//DBHA
}//VideoCloud
}//Dahua


#endif //__DAHUA_VIDEO_CLOUD_DB_HA_SERVER_H__
