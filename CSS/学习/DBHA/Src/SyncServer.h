//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:    主备同步模块服务端,服务端运行在主机上.
//	Revisions:		2017-1-5  mao_jijia
//

#ifndef __DAHUA_VIDEO_CLOUD_DBHA_SYNCSERVER_H__
#define __DAHUA_VIDEO_CLOUD_DBHA_SYNCSERVER_H__

#include "SyncConnect.h"
#include "Common/Server.h"
#include "Infra/Thread.h"
#include "Infra/Semaphore.h"
#include "Waiter.h"

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

class CSyncServer : public EFS::CServer, public Infra::CThread
{
public:
	CSyncServer();
	~CSyncServer( void );

	bool startup();
	bool cleanup();

	//是否在同步中.
	bool isSyncing();

	//设置mysqlserverID
	void setMysqlServerID(std::string serverID);
private:
	//接收到连接上来.
	virtual int accept( NetFramework::CSockStream& sock );
	//同步连接断线.
	void closeConnect();

	//接收到网络包.
	void onPacket( EFS::TPacketBasePtr req, EFS::TPacketBasePtr& rsp ); 
	//异步发送到对端,记录发送出去的命令.
    bool sendToSlave( EFS::TPacketBasePtr req );
	//等待备机返回.与sendToSlave必须匹配使用.
	bool waitSlaveResponse( uint32_t timeout );

	//同步相关操作.
	virtual void threadProc();
	bool getDumpFileInfo();
	//mysql dump文件传输相关
	int32_t getMasterFile();
	// 当主机的备份文件无效的时候，直接删除当前的备份文件
	bool cleanMasterFile();
	// 解析master的dump文件信息
	bool parseDumpInfo( bool flag);
	//备机来获取主机dump文件如果不存在，产生一个
	bool mysqlMasterDump();
	// 停止dump文件的产生
	bool stopMysqlDump();
	//检测备份线程是否在运行
	bool checkmysqlMasterDump();

	bool beginSync();
	bool postData( char* data, int32_t len );
	bool postFile();
	bool endSync();

private:

	bool m_is_started;
	bool m_is_init_server; //服务是否开启,CServer关闭以后就销毁,不能复用,所以只启动一次.
	CSyncConnect* m_connect_ptr;
	Infra::CMutex m_connect_mutex;
	Infra::CSemaphore* m_close_sem;
	
	enum SyncState{
		noSync, //备机还没有上线,还没有比较过版本号.
		synced, //同步.
		unsynced, //不同步.
	};
	SyncState m_sync_state;
	bool m_is_syncing;
	
	//主备同步模块都是同步的消息,所以没有用队列方式.
	int32_t m_cmd_type; //消息类型.
	int8_t m_is_error; //是否错误.
	int32_t m_error_type; //错误类型.
	Infra::CSemaphore* m_wait_sem;

	//mysqlServerID
	std::string m_server_id;
	bool m_connect;	//退出标记

	std::string m_slave_recovery_file_name; // 备机上历史恢复的文件名
	uint64_t m_slave_recovery_file_len;     // 备机上历史恢复的文件的位置
	std::string m_master_dump_full_file;    // 主机上获取到的备份文件,绝对路径
	std::string m_master_dump_binlog_file;
	int64_t m_master_dump_binlog_pos;
	std::string m_master_dump_md5sum;
	uint64_t m_master_dump_file_pos;		// 主机需要偏移到文件的位置
	std::string m_master_dump_file_name;	// 主机上获取到的备份文件名
	std::string m_pre_master_dump_file_name;// 前一次获取到的主机备份文件的名字
	int32_t	m_pre_dump_file_cnt;			// 前一次主机备份文件的使用次数
};

} // DBHA
} // VideoCloud
} // Dahua

#endif // __DAHUA_VIDEO_CLOUD_DBHA_SYNCSERVER_H__


