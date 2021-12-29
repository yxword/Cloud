//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:    主备同步模块客户端,客户端运行在备机上.
//	Revisions:		2017-1-5  mao_jijia
//

#ifndef __DAHUA_VIDEOCLOUD_DBHA_SYNCCLIENT_H__
#define __DAHUA_VIDEOCLOUD_DBHA_SYNCCLIENT_H__

#include "HAMonitor.h"
#include "Infra/Timer.h"
#include "Common/Stream.h"
#include "Memory/SharedPtr.h"
#include "Infra/File.h"

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

class CSyncClient
{
public:
	CSyncClient( );
	~CSyncClient( void );

	//开始,成为备机时候会调用.
	bool startup( );

	//结束,成为主机时候或者主动停止时候调用.
	bool cleanup();

	//连接发生改变时候会调用.
	void resetLink();

	//是否在同步中.
	int32_t getSyncState();

	//阻塞等待
	void waitForDumping();
	void wakeupDumping();

	void getRecoverBinlogInfo(std::string &file, int64_t &pos, std::string &md5sum){
		file = m_current_recover_binlog_file;
		pos = m_current_recover_binlog_pos;
		md5sum = m_current_recover_md5sum;
	}

	bool checkRecoverFileMd5sum(std::string dump_file_name, std::string& md5sum);
private:
	//流注册和心跳.
	void onStreamTimer( unsigned long );

	//链接到主机.
	bool connect();

	//主动断线,切换链路.
	bool disconnect();

	//发送心跳.
	bool heartbeat();

	//链路状态回调.
	void onStreamState( int type, EFS::TPacketBasePtr pkt );

	//处理协议包.
	void onPacket( EFS::TPacketBasePtr pkt );

	void getRecoveryFile(std::string& fileName, uint64_t& len);
	EFS::TPacketBasePtr onDumpFileInfoPacket( EFS::TPacketBasePtr msg );
	EFS::TPacketBasePtr onSyncBeginPacket( EFS::TPacketBasePtr msg );
	EFS::TPacketBasePtr onDataSyncPacket( EFS::TPacketBasePtr msg );
	EFS::TPacketBasePtr onSyncEndPacket( EFS::TPacketBasePtr msg );

public:
	enum SyncState{
		offline, //离线状态.
		unsynced, //没有同步.
		inSyncing, //同步中.
		syncError, //状态发生错误.
		synced //已经同步.
	};
std::string m_master_dumpfile_md5;  //主机传输过来的dumpfile的md5值
private:

	bool m_is_started;	// 是否startup的标记.
	bool m_exit;		// 退出标记

	// 用于同步和唤醒
	pthread_mutex_t m_dumping_mutex;
	pthread_cond_t m_dumping_cond;
	int32_t m_wait_dump_threads;

    EFS::CStream* m_stream;
	Infra::CMutex m_close_mutex;
	Infra::CSemaphore* m_close_sem;

	std::string m_server_ip;
	std::string m_local_ip;
	Infra::CTimer m_timer; //心跳和注册定时器.
	uint32_t m_heartbeat_timeout;

	Infra::CFile m_file;

	SyncState m_sync_state;
	std::string m_pre_recovery_file_name;  // 备机上原先的恢复文件名称
	uint64_t m_pre_recovery_file_size;	   // 备机原先恢复文件的位置
	std::string m_current_recovery_file;   // 备机当前恢复的原始名字
	std::string m_current_recover_binlog_file;
	int64_t m_current_recover_binlog_pos;
	std::string m_current_recover_md5sum;
};

} // DBHA
} // VideoCloud
} // Dahua

#endif // __DAHUA_VIDEOCLOUD_DBHA_SYNCCLIENT_H__

