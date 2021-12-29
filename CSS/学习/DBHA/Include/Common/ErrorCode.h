//
//  "$Id$"
//
//  Copyright (c)1992-2011, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_ERROR_CODE_H__
#define __INCLUDE_DAHUA_EFS_ERROR_CODE_H__

namespace Dahua{
namespace EFS{

enum ErrorCode{
 	efsSucceed              = 0, // 成功
 	
	// SDK层错误
	efsFail                 = -10000,            // 失败 
    efsReachFileEnd         = -10001,            // 到达文件尾 
    efsInvalidFile          = -10002,            // 文件无效 
    efsParamError           = -10003,            // 参数错误 
    efsModeError            = -10004,            // 文件模式错误 
    efsUnsupported          = -10005,            // 操作不支持 
    efsInvalidOffset        = -10006,            // offset非法 
    efsMaxClientLimit       = -10007,            // 最大客户端连接数 
    efsIsNotEmpty           = -10008,            // efs系统当前不为空
    efsInvalidBucket        = -10009,            // bucket无效
    efsCommunicateMdsError  = -10010,			 // EOSClient与mds通讯异常
    efsClientError			= -10011,			 // EOSClient内部错误
    efsDnErrorLimit			= -10012,			 // 最大DN错误数，N+M情况下，发生了大于M个错误
    efsClientNotReady		= -10013,			 // SDK没有准备就绪，发生在错误的调用上，如没有初始化就调用其他接口
    efsErasureCodeError		= -10014,			 // EC算法错误
    efsMetadataError		= -10015,			 // 元数据错误	
    efsCommunicateCatalogServerError	= -10016,	// EFSClient与catalogServer通讯异常	   
    efsSmallFileSizeLimited	= -10017,			 // 超过小文件最大大小
    efsFileBeTruncated		= -10018,			 // 文件被截断，发生在数据已经写到dn，但sdk或mds截断文件的情况
    efsOutOfFileRange		= -10019,			 // 操作超出文件范围，如seek传入参数大于文件范围
	efsContainersLimit 		= -10020,			 // container达到最大并发限制
	efsCommunicateDNError	= -10021,			 // 与dn通讯异常
	efsSystemFdLimit		= -10022,			 // 系统句柄受限导致错误
	efsOperationNeedRetry   = -10023,            // 操作失败，建议进行重试
	
	// RPC层错误
	efsAddressError			= -20000,			// 网络地址错误
	efsNetworkError 		= -20001,			// 正常连接出现异常
	efsBindFailed			= -20002,			// server绑定监听失败
	efsAcceptError			= -20003,			// 接受连接异常
	efsConnectError			= -20004,			// 发起连接时错误
	efsConnectTimeout		= -20005,			// 连接超时
	efsMessageParseError	= -20006,			// 协议解析错误
	efsWaitResponseTimeout	= -20007,			// 请求超时时间内得不到回复
	efsMessageNotSupport	= -20008,			// 请求的消息不支持，即没有消息处理函数
	efsNetworkAgain         = -20009,           // 请求暂时不可发送，可重试 
	
	// Linux文件系统错误
	efsOpenFileError	 	= -30000,			// 打开文件错误
	efsReadFileError		= -30001,			// 读文件错误
	efsWriteFileError		= -30002,			// 写文件错误
	efsFileAlreadyUsed		= -30003,			// 文件忙
	efsLinkFileError		= -30004,			// link错误
	efsUnlinkFileError		= -30005,			// unlink错误
	efsCloseFileError		= -30006,			// 关闭文件错误
	efsFileSystemError		= -30007,			// 文件系统错误
	efsStatFileError		= -30008,			// 获取文件属性错误
	efsMakeDirError			= -30009,			// 创建目录错误
	efsAccessDirError		= -30010,			// 目录访问错误
	efsMountError			= -30011,			// 挂载错误
	efsInvalidFD			= -30012,			// 无效的fd
	efsSeekFileError		= -30013,			// seek文件失败
	efsSyncFileError		= -30014,			// 同步文件到磁盘错误
	efsSysFileNumLimitedError  = -30015,        // EMFILE,Too many open files/ ENFILE  File table overflow
	efsTruncateFileError	= -30016,           // 文件截断错误
	
	// EFS对象层错误
	efsObjectCollectionsNumLimited		= -40000,			// object collection个数超过系统限制
	efsObjectCollectionSizeLimited		= -40001,			// object collection大小超过系统限制
	efsObjectCollectionTypeNotSupport	= -40002, 			// 文件类型不支持
	efsECRuleNotSupport					= -40003,			// 冗余规则不支持
	efsObjectCollectionModeNotSupport	= -40004,			// object collection打开模式不支持
	efsObjectCollectionBusy				= -40005, 			// object collection处于写打开状态
	efsObjectCollectionNotWriting		= -40006, 			// object collection不处于写状态
	efsObjectCollectionExists			= -40007,			// object collection已经存在
	efsObjectCollectionNotExists		= -40008,		 	// object collection不存在
	efsObjectCollectionLeaseNotExists	= -40009, 			// object collection租约不存在
	efsObjectNumLimited					= -40010, 			// Object分配失败，Object个数过多
	efsHandlingObjectsLimited	 		= -40011,			// Object分配失败，未修复的Object过多
	efsObjectConstructionTimeout 		= -40012,			// 对象构造超时
	efsObjectNotExists			 		= -40013,			// 对象不存在
	efsDNResourcePoor		 	 		= -40014,			// DN资源不足，导致object分配失败
    efsObjectBucketNotExists      		= -40015,           // object bucket不存在
	efsObjectBucketNotEmpty       		= -40016,           // object bucket非空
    efsObjectBucketBusy	        		= -40017,           // object bucket正在使用中
    efsObjectBucketsNumLimited			= -40018,			// object bucket数目超出系统现在
    efsSystemCapacityPoor				= -40019,			// 系统容量不足
    efsObjectBucketExists				= -40020,			// object bucket已经存在
    efsDNResourceNotReady				= -40021,			// 刚提供服务时，dn还未准备就绪，需要sdk重试以避免错误
	efsNetworkNotMapping				= -40022,			// 网闸没有建立映射关系
	efsTooManyOpenObjectCollections		= -40023,			// 一个客户端打开的文件数过多
	efsEmptyObjectCollectionsNumLimited	= -40024,			//一个客户端在配额不足的情况下连续创建的空文件数过多
	efsObjectCollectionBroken			= -40025,			//文件已损坏
	efsObjectNeedReportSizeLimited		= -40026,			//该汇报的object超过限制,溢出
    	
	// 逻辑错误
	efsVersionNotSupport	= -50000,			// 版本不支持
	efsParseImageError		= -50001,			// fsimage解析错误
	efsParseEditlogError	= -50002,			// editlog解析错误
	efsSessionExists		= -50003,			// 会话已经存在，不允许重复建立
	efsClusterIdError		= -50004,			// 集群 uid错误
	efsDatanodIdError		= -50005,			// Datanode id错误
	efsSystemInfoBroken		= -50006,			// 系统信息不完整
	efsExecShellError		= -50007,			// 执行shell错误
	efsParseShellError 		= -50008,			// shell结果解析错误
	efsParseSystemResourceError = -50009,		// 解析系统资源错误
	efsPointerNull			= -50010,			// 传递进来的指针是NULL
	efsHAXidError 			= -50011,		// xid相关错误
	efsHAOpenFileError 		= -50012,			//open failed
	efsHAOffsetError  		= -50013,
	efsHAReadError 			= -50014,
	efsHAWriteError 		= -50015,
	efsHAGetFileError 		= -50016,
	efsHARenameError 		= -50017,
	efsHASubscibeEditlogError	= -50018,
	efsHAReSubscibeEditlogError	= -50019,
	efsParseConfigError		= -50020, 			// 解析配置错误
	efsConfigItemNotExists  = -50021, 			// 配置项不存在
	efsConfigItemInvalid	= -50022,			// 配置项无效或为空
    efsDNDeleting           = -50023,           // Datanode正在删除中，不允许注册Datanode。

	// 权限管理错误
	efsUserNotExists		= -60000,			// 用户不存在
	efsPasswordError		= -60001,			// 用户密码错误
	efsDNPermissionDenied	= -60002,			// DN不合法
	efsClientNameInvalid    = -60003,           // ClientName非法。
	efsLicenseDenied		= -60004,			// License不合法
	efsLicenseCapLimited	= -60005,			// 系统容量受到License限制
	efsMdsStateUnallowed	= -60006,			// 当前MDS状态不允许此操作
	efsUserExists			= -60007,			// 用户已经存在
	efsUserNameError		= -60008,			// 用户名非法
	efsUserPermissionDenied = -60009,			// 用户操作无权限
	efsUserBucketNumLimited = -60010,			// 用户bucket个数上限错误，设置时达到用户上限，或者创建时达到用户配额
	efsUserBucketNumOverflow= -60011,			// 用户bucket个数配额小于实际值
	efsUserBucketCapacityOverflow = -60012,		// 用户bucket容量配额小于实际值
	efsUserDataNotEmpty		= -60013,			// 用户数据非空，即存在bucket，此时不允许删除用户
	efsNetBandWidthLimited	= -60014,			// 网络带宽限制
	efsDiskIOBandWidthLimited= -60015,			// 数据磁盘IO带宽限制
	
	// Datanode Block错误
	efsBlockAlreadyPrepared = -70000,			// prepare阶段,发现当前session已经在写数据
	efsBlockNotPrepared		= -70001,			// 在write block时,文件都没有准备过
	efsBlockIndexExists		= -70002,			// 在write block或read block时,块索引冲突
	efsBlockIndexNotExists	= -70003,			// 块索引不存在
	efsBlockSizeLimited		= -70004,			// 块写入长度溢出
	efsWriteOffsetError		= -70005,			// 出现非追加写
	efsReadOffsetError		= -70006,			// 读偏移超过文件长度
	efsAllocBlockError		= -70007,			// block分配错误，空间不足
	efsBlockIndexInvalid	= -70008,			// 无效的块索引
	efsVolumeListEmpty		= -70009,			// 磁盘列表为空
	efsAllVolumeSpacePoor	= -70010,			// 所有磁盘的可用空间都不足
	efsDatanodeIsExiting    = -70011,           // Datanode收到退出信号,准备退出
	efsUpdateBlockStatError = -70012,            // 设置块属性错误
	efsBlockSizeNotAllowed  = -70013,           // prepare阶段，blocksize与DN配置文件中的块大小不一致
	efsBlockExceptionClosed = -70014,			// 写阶段的块没有正常关闭，该状态在读到块尾部的时候返回给sdk
	efsAttributeKeyNotExists= -70015,			// 元数据中找不到附加属性的key/value键值对
	efsBlocksNumLimited     = -70016,			// datanode块个数限制
	efsVolumeIsInitializing = -70017,			// datanode存储层正在初始化
	efsBlockRecoveryError   = -70018,           // Block块恢复出错
	efsBlockCheckSumError   = -70019,           // Block块校验值错误
	efsBlockRecoverWaitTimeout = -70020,        // 块恢复,任务在recoverManager队列中就已超时
	efsDatanodeError        = -70021,           // Datanode 内部错误
	efsExceedMsgQueueLimit  = -70022,           // 消息队列已满.SDK端应该重试
	efsRedoCheckSumError    = -70023,           // truncate或覆盖写block尾部数据时,无法重新计算尾部chkSumSize不对齐部分的CRC

	// 文件层错误
	efsFileNameInvalid      = -80001,           // 文件名称不合法
    efsFileNotExists  		= -80002,           // 文件不存在
    efsFileExists			= -80003,           // 文件已经存在  
    efsBucketNotExists		= -80004,			// Bucket不存在
    efsBucketExists			= -80005,			// Bucket已经存在
    efsBucketNotEmpty		= -80006,			// Bucket不为空 
    efsBucketNameInvalid	= -80007,			// Bucket名称不合法
    efsFileContainerNotExists = -80008,			// 文件容器不存在
    efsCatalogServerError	= -80009,			// 目录服务内部错误
    efsMetaServiceNotMatching = -80010,			// catalogserver和mds不匹配	
	efsFileNumLimited		  = -80011,			// 文件数达到系统上限
	efsFileLocked			= -80012, 			// 文件被锁定

	// 管理者工作者服务-框架错误.
	efsWorkerBusy			= -90001,			//工作者无空闲（队列已满）.
	efsWorkerInvalidParam	= -90002,			//无效的参数.
	efsWorkerStartFailed	= -90003,			//任务启动失败.
	efsManagerBusy			= -90004,			//管理者无空闲（队列已满）
	efsTaskNotExists		= -90005,			//任务不存在
	efsTaskParseError		= -90006,			//任务信息解析失败
	efsTaskTypeUnsupported	= -90007,			//任务类型不支持
	efsTaskProcessTimeout	= -90008,			//任务处理超时
	
	// 管理者工作者服务-淡忘操作错误.
	efsForgottenStorageRecordNotSupportSVC	= -90101,	//H264录像文件不支持SVC淡忘.
	efsForgottenStorageCheckMidFileFailed	= -90102,	//淡忘校验中间文件失败.
	efsForgottenStorageExtractFramefailed	= -90103,	//淡忘解析帧失败.

	//代理服务器错误码
	efsProxyBufferFullError = -91001,                   //代理与EFS间的连接缓冲区已满
	efsProxyToEfsNetworkError = -91002,                 //代理到CS或MDS间的网络错误
	efsProxyInternalError  = -91003,                	//代理服务器内部错误
	efsProxyBucketListNotReady = -91004,                //代理第一次汇聚bucket未结束，SDK客户端需要重试
	efsProxyBucketAndFileHashKeyMatchFailed = -91005    //根据bucketName或fileId计算出来的hash key值和efs作用范围内的hash值不匹配
};

} //namespace EFS
} //namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_ERROR_CODE_H__
