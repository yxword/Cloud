//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:    消息类型定义.
//	Revisions:		2016-12-3  mao_jijia
//

#ifndef __DAHUA_VIDEOCLOUD_DBHA_MESSAGETYPE_H__
#define __DAHUA_VIDEOCLOUD_DBHA_MESSAGETYPE_H__

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

enum MessageType
{
	HAFK_MULTICAST_HEARTBEAT_MESSAGE = 10000,           //组播心跳
	HAFK_JSON_MESSAGE = 10001,                          //Json消息
	//HAFK_ADD_DATANODE_MESSAGE = 10002,					//同步主备数据节点的信息.
	//HAFK_DEL_DATANODE_MESSAGE = 10003,					//同步主备数据节点的信息.
	//HAFK_DATANODE_MAP_MESSAGE = 10004,					//同步主备数据节点的信息.
	HAFK_SYNC_DUMP_FILE_INFO_MESSAGE = 10005,           //同步, 获取备机上的文件信息
	HAFK_SYNC_BEGIN_MESSAGE = 10006,                    //同步开始
	HAFK_SYNC_DATA_MESSAGE = 10007,                     //同步数据传输
	HAFK_SYNC_END_MESSAGE = 10008,                      //同步结束
	HAFK_HEARTBEAT_MESSAGE = 10009,                     //主备心跳, TCP连接
	HAFK_SYNC_NOTIFY_RECOVER_FINISH = 10010,            //备机恢复结束
	//DNFK_HEARTBEAT_MESSAGE = 10010,                     //子节点心跳, TCP连接
	//DNFK_DATANODE_CONNECT_MESSAGE = 10011,				//
};

} // DBHA
} // VideoCloud
} // Dahua

#endif // __DAHUA_VIDEOCLOUD_DBHA_MESSAGETYPE_H__
