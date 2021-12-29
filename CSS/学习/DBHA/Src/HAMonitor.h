//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __DAHUA_VIDEOCLOUD_DBHA_HAMONITOR_H__
#define __DAHUA_VIDEOCLOUD_DBHA_HAMONITOR_H__

#include "HeartbeatMonitor.h"
#include <string>
#include "Infra/Mutex.h"

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

typedef std::map<CHeartbeatMonitor::JudgerType, std::list<CHeartbeatMonitor::Judger> > JudgerMap;

class CHAMonitor
{
public:
    CHAMonitor( void );
    ~CHAMonitor( void );

    //开启心跳模块.
	//该处的锁主要是为了防止状态变化时，程序退出，将状态函数置为空
    bool startup( CHeartbeatMonitor::HAStateProc3 proc,   JudgerMap judgers );
	
    //停止心跳模块.
    bool cleanup();

protected:

    //获得心跳参数.
    CHeartbeatMonitor::HeartbeatArgs getHeartbeatArgs();

    //心跳状态发生变化.
    bool handleHAStateChange( CHeartbeatMonitor::HAAction action, CHeartbeatMonitor::LinkState& linkState);

    //获取心跳协议包,用来生成uid和id.
    void handleHeartbeatMsg( CHeartbeatMonitor::HAState localState,
                             CHeartbeatMonitor::HAState remoteState,
                             TMulticastHeatBeatMsgPtr msg );

private:
    std::string createUID();
    uint8_t createClusterID();
    void timeSlotProc( int64_t id );
    void setHeartBeatArgs();
    bool mysqlGetCurrentBinlog(std::string &binlogName, int32_t &binlogPos);
private:
    CHeartbeatMonitor::HAStateProc3 m_ha_state_proc; //上层链路状态回调.
    CHeartbeatMonitor* m_heartbeat_monitor;
    Infra::CMutex m_mutex;                          //

    Infra::CReadWriteMutex m_hb_mutex;
    CHeartbeatMonitor::HeartbeatArgs m_hb_args;
    int64_t m_timer_id;; //信息获取定时器
};

} // DBHA
} // VideoCloud
} // Dahua

#endif // __DAHUA_VIDEOCLOUD_DBHA_HAMONITOR_H__
