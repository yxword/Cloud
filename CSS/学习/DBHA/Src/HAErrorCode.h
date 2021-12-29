#ifndef __DAHUA_VIDEOCLOUD_DBHA_HAERRORCODE_H__
#define __DAHUA_VIDEOCLOUD_DBHA_HAERRORCODE_H__

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

enum HAErrorCode
{
	//主备同步错误码.
	noSlaveOnline = 1000,			//没有备机在线.
	sendToSlaveFailed,				//发送到备机失败.
	waitSlaveResposeTimeout,		//等待备机响应超时.
	jsonMessageInvalid,				//非法的json串.
	sqlCmdTypeInvalid,				//非法的数据库操作类型.
	sqlOptFailed,					//数据库操作返回失败.
	sqlCloseFailed,					//数据库关闭失败.
	sqlOpenFailed,					//数据库打开失败.
	sqlGetFileNameFailed,			//获取数据库文件名失败.
	sqlOpenTmpFileFailed,			//打开临时数据库失败.
	sqlTmpFileError,				//数据库临时文件发生错误.
	sqlIsSyncing,					//数据库在同步中.
	sqlUnsynced,					//数据库还没有同步.
	configSaveFailed,				//配置保存失败.
	renameFileFailed,				//重名名文件失败.
	removeFileFailed,				//删除文件失败.
	networkError,                   //网络错误.

	//管理节点与子节点错误码.
	waitDataNodeResponseTimeout,	//等待管理节点或者子节点响应错误码.
	dataNodeCmdTypeInvalid,			//非法的管理节点与子节点操作类型.
	dataNodeOptFailed,				//节点间操作返回失败.
	resJsonMessageInvalid,			//非法的响应JSON串.
	dataNodeHBProFailed,			//心跳响应回调失败.
	dataNodeHBGenMsgFailed,			//生成心跳响应信息失败.
	dataNodeGernalHBMsgFailed,			//处理通用心跳响应信息失败.
	dataNodeHBSetMsgFailed,			//设置心跳响应信息失败.
	dataNodeIdGetFailed,			//获取DataNode ID失败.
	dataNodeSessionIsExist, 		//链接已存在.
	dataNodeClusterIdGetFailed,			//获取Cluster ID失败.
	dataNodeClusterIdError,			//集群UID错误.
	dumpFileMd5CheckFailed,				//传输至备机的dump文件MD5验证失败
};

} // DBHA
} // VideoCloud
} // Dahua

#endif // __DAHUA_VIDEOCLOUD_DBHA_HAERRORCODE_H__

