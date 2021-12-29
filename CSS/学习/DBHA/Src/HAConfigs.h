#ifndef __DAHUA_VIDEOCLOUD_DBHA_HACONFIG_H__
#define __DAHUA_VIDEOCLOUD_DBHA_HACONFIG_H__

#include <string>
#include <map>
#include "Infra/IntTypes.h"
#include "Infra/Mutex.h"
#include "Json/json.h"
#include "DBHAConfig.h"
#include "Infra/Timer.h"



namespace Dahua {
namespace VideoCloud {
namespace DBHA {

class CHAConfigs
{
public:
    CHAConfigs( void );
    ~CHAConfigs( void );

    static CHAConfigs& instance();

    //设置工作目录
    void setWorkspace( const std::string& path );

    void setHAConfig( HAConfig& config );

    //保存配置文件
    bool saveConfig();

	//加载配置文件.
	bool loadConfig();
	//加载HA状态
	bool loadHAState();

    //保存HA状态
    bool saveHAState( int32_t usedState, int32_t masterTimestamp );

//////////////////////////////////////////////////////////////////////////

    const std::string getUId();
    void setUId( const std::string uid );

    uint8_t getClusterId();
    void setClusterId( uint8_t clusterId );

    uint32_t getMulticastTimeInterval();
    uint32_t getMasterDownInterval();

    int32_t getUsedState();
    int32_t getMasterTimestamp();

    uint32_t getSyncServerPort();
    uint32_t getSyncHeartbeatInterval();
    uint32_t getSyncHeartbeatTimeout();
    uint32_t getSyncConnectTimeout();
    uint32_t getSyncCallTimeout();
    uint32_t getMasterWaitSlaveTime();

    uint64_t getSyncVersion();
    void increaseSyncVersion();
    void setSyncVersion( uint64_t version );

    //HAConfig
    std::string getMulticastIP();                //组播地址
    uint32_t getMulticastPort();                 //组播端口
    std::string getLocalHeartbeatIP();           //本机心跳IP
    std::string getPeerHeartbeatIP();            //对端心跳IP
    std::string getLocalBusinessIP();            //本机业务IP
    std::string getPeerBusinessIP();             //对端业务IP
    std::string getVIP();                        //虚IP
    std::string getVIPNetmask();                 //虚IP掩码
    std::string getVIFName();                    //虚网卡名, 比如bond0:0
    std::string getIFName();                     //真实网卡名, 比如bond0, 用于arping
	std::string getHeartbeatIFName();			 //真实心跳网卡名，如bond1，用于arping
    std::string getServiceName();                //服务名称, 用于区分不同集群
    std::string getUserDefined();                //用户自定义字段, 集群匹配时会使用.

//////////////////////////////////////////////////////////////////////////

	bool loadSqlState(); //导入mysql数据库

    bool saveSqlState(); // 保存mysql数据库状态信息

	bool setSQLWriteState(int state); // 设置可读可写模式
	bool newMachine(); // 检测是否是新机器

	void getInitBinlogInfo(std::string& fileName, int32_t& postion); // 获取init binlog信息

	//设置初始化binlog信息
	bool setInitBinlogInfo(std::string binlogInfo );

	//备机切换为主机设置binlog位置信息
	//bool setChangeMasterInfo(std::string binlogInfo,int32_t flag);

	void getChangeMasterInfo(int32_t& flag, std::string& fileName, int32_t& postion); // 获取slave变成master信息
	void setChangeMasterInfo(const std::string& localBinlogInfo, int32_t localPosition);
	bool getSlaveConnectMasterState(); // 获取备机和主机的连接状态
	bool getSlaveChangeMasterFlag();   // 获取备机变成主机的合法检测项

	bool setSlaveConnectState(std::string connectState);// 设置slave连接状态
	bool setSlaveSyncState(std::string syncInfo);	 // 设置slave同步状态
	std::string getSlaveConnectState();				 // 获取slave连接状态
	std::string getSlaveSyncState();				 // 获取slave同步状态

	//设置当前主备模式
	bool setSQLMode(int mode);

	//获取当前mysql主备模式
	int32_t getSQLMode();

	// 设置DB是单机模式还是双机模式，true表示双机，false表示单机
	///*void setSingleMode(bool flag=true);*/

private:

    bool saveToJsonFile( const Json::Value& root, const std::string& fileName, bool safety );
    bool loadFromJsonFile( const std::string& fileName, Json::Value& value );

    bool writeToFile( const std::string& fileName, const std::string& content );
    bool readFromFile( const std::string& fileName, std::string& content );

public:
	enum SQLWriteState{
		NotReadNotWrite=0,
		ReadState=1,
		WriteState=2,
	};
	enum SQLMode{
		NotSlaveNotMaster=-1,
		Slave=0,
		Master=1,
	};

private:

    // 心跳模块相关的配置
    // 以下配置文件加载
    std::string m_uid;                        // device uid
    uint8_t m_cluster_id;                     // cluster id
    uint32_t m_master_wait_salve_time;         // 单位: 秒, 启动时等待salve启动的时间
    uint32_t m_multicast_time_interval_ms;     // 组播间隔.
    uint32_t m_master_down_interval_ms;        // 单位: 毫秒, 监测主备存在的间隔时间

    // HA状态
    int32_t m_used_state;                     // 本机历史状态.
    int32_t m_master_timestamp;               // 本机成为master的时间戳.


    // 同步模块的配置
    // 以下从配置文件加载
    uint32_t m_sync_heartbeat_interval_ms;    //单位: 毫秒, 主备之间tcp链路心跳间隔
    uint32_t m_sync_heartbeat_timeout_num;    //单位: 次数, 心跳超时次数
    uint32_t m_sync_connect_timeout;          //单位: 毫秒, 连接超时时间
    uint32_t m_sync_call_timeout;             //单位: 毫秒, 调用超时时间
    uint64_t m_sync_version;                  //版本号


	bool m_new_machine_flag;		// 是否是全新机器的标记
	int32_t m_SQL_write_state;      // mysql读写标记

	std::string m_SQL_init_binlog_file; // 初始化的主机文件名
	int32_t m_SQL_init_binlog_position; // 初始化的主机位置

	int32_t m_master_change_flag;			     // 备机成为主机的合法性标记
	std::string m_master_change_binlog_file; // 备机切换为主机的文件名
	int32_t m_master_change_binlog_postion; // 备机切换为主机的文件位置

	//数据库成为主机状态
	//int32_t m_slave_IO_thread;		     //mysql sql线程是否ok/mysql io线程是否ok
	//int32_t m_slave_sql_thread;			//备机是否已经与主机同步上判断binlog位置

	std::string m_slave_sync_info;		// 同步标记
	std::string m_slave_connect_master;     //mysql备机检测主机是否在线状态

	int32_t m_SQL_mode;//mysql主备当前状态:-1非主非备,0备机，1主机

private:
    std::string m_workspace;        //配置文件路径

    Infra::CMutex m_configFileMutex;
    Json::Value m_configRoot;       //HA Config

    Infra::CMutex m_stateFileMutex;
    Json::Value m_stateRoot;        //State

	Infra::CMutex m_sqlFileMutex;	//sql状态读锁因为changtomaster与定时检测mysql服务是否运行都会调用该函数
	Json::Value m_sqlRoot;			//sql state 

    HAConfig m_HAConfig;
};

#define gHAConfig CHAConfigs::instance()

} // DBHA
} // VideoCloud
} // Dahua

#endif // __DAHUA_VIDEOCLOUD_DBHA_HACONFIG_H__

