//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_PROTOCOL_H__
#define __INCLUDE_DAHUA_EFS_PROTOCOL_H__

namespace Dahua {
namespace EFS {
//=========================通用=======================
/*协议头
字节序 1   2       3   4       5   6   7   8
    Version 协议类 协议标识    Sequence

字节序 9   10      11  12      13  14  15  16
    bodylength    保留字      ‘D’    ‘H’ ‘C’ ‘S’

字节序 17  18  19  20  21  22  23  …
        Body
*/


//协议版本
//第一版协议,面向大文件在集群中使用ec方式存储
#define  PROTO_VERSION_V1 0x01

//协议族
enum    ProtocolType{
    DS_PROTOCOL_REQ=0x10,  //大华存储协议-请求类协议
    DS_PROTOCOL_RES=0x20,  //大华存储协议-响应类协议
    DS_PROTOCOL_NOTIFY=0x30//大华存储协议-通告类协议
};


//请求响应式消息类型,即发起端是请求类协议,接收端是响应类协议
enum MessageType{

    /*
        对象客户端请求MDS,MDS响应请求给对象客户端
    */
    //裸object相关的操作
    OC2MDS_CREATE_OBJECT_COLLECTION_MESSAGE = 1001,         //申请创建collection,带有表示申请哪类collection,响应有内部编码过的file id

    OC2MDS_APPEND_OBJECT_COLLECTION_MESSAGE = 1002,         //插入写,返回collection的最后一个LocatedObject
             
    OC2MDS_ALLOC_STORAGE_MESSAGE = 1003,                    //申请存储对象带排除的datanode,返回LocatedObject

    OC2MDS_CLOSE_OBJECT_COLLECTION_MESSAGE = 1004,          //关闭collection，关闭时指定写到第几个object,mds释放租约
                                                       
    OC2MDS_GET_OBJECT_LOCATIONS_MESSAGE = 1005,             //传递collection的key,offset,long ,返回LocatedObjects
                                                            //客户端从数据节点更新最后一个未确定状态的object
    OC2MDS_DELETE_OBJECT_COLLECTION_MESSAGE = 1006,         //申请删除文件,后续MDS会负责将真正删除发送给DATANODE
            
    OC2MDS_GET_OBJECT_COLLECTION_INFO_MESSAGE = 1007,       //返回collection status,不包括LocatedBlocks信息
   
    OC2MDS_REQUEST_LOGIN_MESSAGE = 1008,                    //新增支持token登录；将clientname和用户名传递给mds，验证客户端是否合法

    OC2MDS_GET_SYSTEM_STAT_MESSAGE = 1009,                  //获取集群的统计

    OC2MDS_HEARTBEAT_MESSAGE = 1010,                        //对象客户端到mds的心跳

    //桶相关的操作
    OC2MDS_CREATE_OBJECT_BUCKET_MESSAGE = 1011,             //创建对象桶
    
    OC2MDS_REMOVE_OBJECT_BUCKET_MESSAGE = 1012,             //删除对象桶
    
    OC2MDS_FIND_OBJECT_BUCKET_MESSAGE = 1013,               //查找对象桶是否存在

    OC2MDS_SET_OBJECT_BUCKET_CAPACITY_MESSAGE = 1014,       //设置桶的容量 
 
    OC2MDS_GET_OBJECT_BUCKET_CAPACITY_MESSAGE = 1015,       //获取桶的容量


    OC2MDS_ALLOC_CONTAINER_MESSAGE = 1016,                  //申请container,返回container基本信息，后续写入则需要走allocstorage协议     

    OC2MDS_GET_SUPPORT_EC_MESSAGE = 1017,                   //获取mds支持的ec type

    /*
        通用文件客户端请求目录服务器，目录服务器响应文件客户端
    */
    FC2CS_CREATE_FILE_MESSAGE = 21001,                      //需要先拿到对象id后，告知目录服务器创建了一个桶内的文件，包含文件名->对象id的映射关系，文件属性信息，在后续的close会提交

    FC2CS_APPEND_FILE_MESSAGE = 21002,                      //append修改文件，更新目录服务器中的大文件的信息

    FC2CS_CLOSE_FILE_MESSAGE = 21003,                       //申请关闭桶内的文件，更新长度信息，目录服务器再更新最后修改时间

    FC2CS_DELETE_FILE_MESSAGE = 21004,                      //申请删除桶内的指定文件，文件客户端不区分大小文件

    FC2CS_RENAME_FILE_MESSAGE = 21005,                      //重命名bucket文件，只在目录服务器变更文件名，文件客户端不区分大小文件
    
    FC2CS_OPEN_FILE_MESSAGE = 21006,                        //通过文件名查找对象id，支持返回大小文件两种类型的信息

    //FC2CS_CREATE_SMALL_FILE_MESSAGE = 21007,                //向目录服务器在创建小文件，预先占位防止重名

    FC2CS_UPDATE_FILE_MESSAGE = 21008,                //占位

    //FC2CS_CLOSE_SMALL_FILE_MESSAGE = 21009,                 //关闭小文件，更新之前的小文件的文件名对应的对象id，offset，size等信息

    FC2CS_LOGIN_CATALOG_SERVICE_MESSAGE = 21010,            //获取efs master入口信息,以便后面文件客户端将取得的mdsip设置到对象客户端

    FC2CS_GET_FILE_INFO_MESSAGE = 21011,                    //获取桶内的文件的属性(长度，创建时间，修改时间，文件类型)

    FC2CS_CREATE_BUCKET_MESSAGE = 21012,                    //创建bucket，在对象客户端创建得到bucket id后，将bucket名和id的映射传递给目录服务器

    FC2CS_REMOVE_BUCKET_MESSAGE = 21013,                    //删除bucket，目录服务器中没有bucket内的文件，则将目录服务器中的记录删除掉

    FC2CS_FIND_BUCKET_MESSAGE = 21014,                      //查找bucket，根据bucketname，查找返回的bucketid

    FC2CS_LIST_BUCKET_MESSAGE = 21015,                      //遍历查找bucket名

    FC2CS_LIST_FILE_RANGE_MESSAGE = 21016,                  //通过文件名范围查找出符合的文件列表

    FC2CS_LIST_FILE_REGEXP_MESSAGE = 21017,                 //通过通配前缀以及其他条件，查找符合的文件列表

    FC2CS_HEARTBEAT_MESSAGE = 21018,                        //文件客户端到目录服务器的心跳
	
    FC2CS_LIST_FILE_PREFIX_MESSAGE = 21019,                 //前缀匹配查找符合条件的文件或目录,为fuse做支撑

    FC2CS_SET_BUCKET_CAPACITY_MESSAGE = 21020,				//文件客户端设置bucket容量

    FC2CS_SET_BUCKET_PRIVILEGE_MESSAGE = 21021,				//文件客户端设置bucket权限

    FC2CS_GET_BUCKET_INFO_MESSAGE = 21022,					//文件客户端获取bucket信息:容量，权限，已用容量，所属用户等
	
	FC2CS_SET_FILE_ATTRIBUTE_MESSAGE = 21023,               //文件客户端设置文件自定义属性
	
    FC2CS_GET_FILE_ATTRIBUTE_MESSAGE = 21024,               ////文件客户端获取文件自定义属性

    FC2CS_SET_BUCKET_LIFECYCLE_MESSAGE = 21025,             //bucket使能或去使能循环覆盖

    FC2CS_GET_BUCKET_LIFECYCLE_MESSAGE = 21026,             //获取bucket循环覆盖信息：文件保留天数、空bucket是否删除标记、触发覆盖后的执行动作

    FC2CS_SET_RECYCLE_POLICY_MESSAGE = 21027,                 //用户使能或去使能紧急覆盖

    FC2CS_GET_RECYCLE_POLICY_MESSAGE = 21028,                 //获取用户使能或去使能紧急覆盖标记

    FC2CS_GET_FILE_LOCK_MESSAGE = 21029,					////文件客户端获得文件锁定状态

    FC2CS_SET_FILE_LOCK_MESSAGE = 21030,					////文件客户端设置文件锁定状态

    /*
        目录服务器向mds请求，mds响应结果
    */
    CS2MDS_REGISTER_MESSAGE = 22001,                        //目录服务器向mds注册

    CS2MDS_HEARTBEAT_MESSAGE = 22002,                       //目录服务器和mds之前的心跳

    CS2MDS_PUT_REARRANGE_MESSAGE = 22003,                   //提交要整理的对象id,携带可用的key,value,offset,size，mds接收到认为可处理则返回taskid

    CS2MDS_GET_REARRANGE_PROGRESS_MESSAGE = 22004,          //查找对应的taskid的处理进度

    CS2MDS_DELETE_OBJECT_COLLECTIONS_MESSAGE = 22005,       //删除一批对象

    CS2MDS_REMOVE_BUCKET_MESSAGE = 22006,                   //目录服务器到mds删除bucket
	
    CS2MDS_SET_USER_MESSAGE = 22007,						//向mds设置用户信息：userid,capacity,bucket limit num,用户不存在则创建，存在则更新

    CS2MDS_SET_BUCKET_CAPACITY_MESSAGE = 22008,				//向mds设置bucket容量

    CS2MDS_DELETE_USER_MESSAGE = 22009,						//删除用户	

    CS2MDS_GET_USER_MESSAGE = 22010,						//获取用户信息,容量信息是以mds为准，

    CS2MDS_GET_BUCKET_CAPACITY_MESSAGE = 22011, 			//获取bucket容量信息

    CS2MDS_GET_BUCKET_FILENUM_MESSAGE = 22012,        //获取mds中指定bucket的大文件数、小文件container数
	/*
        目录服务器HA
    */

	CSHA_MULTICAST_HEARTBEAT_MESSAGE = 23001, 				// 目录服务器HA心跳协议
	

    /*
        对象客户端请求DATANODE,DATANODE响应请求给对象客户端
    */
    OC2DN_PREPARE_BLOCK_MESSAGE = 2001,                     //准备block，应该在第一次写入block之前完成交互

    //增加可附带附加信息
    OC2DN_WRITE_BLOCK_MESSAGE = 2002,                       //申请写文件,如果长度为空则表示显示通知数据节点关闭文件

    OC2DN_READ_BLOCK_MESSAGE = 2003,                        //申请读文件,响应读取到的数据

    OC2DN_GET_BLOCK_INFO_MESSAGE = 2004,                    //获取块的状态,与GET_BLOCK_LOCATIONS_MESSAGE有关

    OC2DN_HEARTBEAT_MESSAGE = 2005,                         //客户端向DATANODE发送心跳保活,DATANODE回复ACK

    OC2DN_READ_ATTRIBUTE_MESSAGE = 2006,                    //查询属性中的kv键值对
    /*
        MDS跟随DataNode的心跳或者块汇报所返回的命令
    */
    MDS2DN_DATANODE_COMMAND_MESSAGE = 3001,                 //mds命令消息,内部可有多个命令组成，如下：
                                /*
                                DNA_NULL,           //空回复
                                DNA_UNKNOWN,        //未知命令
                                DNA_REGISTER,       //当mds不存在datanode信息则任何消息到mds，mds都会下发要求dn注册
                                DNA_INVALIDATE,     //无效块，datanode接收到后删除无效块
                                DNA_SHUTDOWN,       //关闭datanode
                                DNA_RECOVERBLOCK,   //块丢失或异常,从其他节点恢复回来
                                DNA_REPORTBLOCK,    //通知datanode从什么位置开始块汇报
                                */
    MDS2DN_REARRANGE_BLOCK_COMMAND_MESSAGE = 3002,          //块整理，根据提供的key,value,offset,size,target block进行整理

    /*
        MDS SLAVE向MDS MASTER请求,MDS MASTER响应MDS SLAVE
    */
    MDSHA_GET_FSIMAGE_MESSAGE = 3101,                       //获取fsimage

    MDSHA_GET_EDITLOG_MESSAGE = 3102,                       //获取editlog

    MDSHA_PUT_FSIMAGE_MESSAGE = 3103,                       //上传fsimage

    MDSHA_GET_FSIMAGE_VERSION_MESSAGE = 3104,               //获取fsimage版本

    MDSHA_PUT_FSIMAGE_VERSION_MESSAGE = 3105,               //推送fsimage版本
    
    MDSHA_ROLL_EDITLOG_MESSAGE = 3106,                      //通知mds停止写入现有日志文件切换到新的日志文件再写入

    MDSHA_SUBSCRIBE_EDITLOG_MESSAGE = 3107,                 //订阅editlog

    MDSHA_SWITCH_FILE_MESSAGE = 3108,                       //通知mds用新的镜像文件替换旧镜像文件,将重命名新日志文件
	
	MDSHA_HEARTBEAT_DETECT_MESSAGE = 3109,                  //主备通过心跳线进行检测是否存活
	
	MDSHA_GET_EDITLOG_INFO_MESSAGE = 3110,                  //获取已经关闭的editlog文件描述信息

    /*
        DATANODE向MDS发送请求,MDS响应DATANODE
    */
    DN2MDS_REGISTER_MESSAGE = 4001,                         //带令牌和版本向元数据注册，元数据返回mds和datanode的描述的string

    DN2MDS_REPORT_BLOCK_MESSAGE = 4002,                     //主动汇报,盘少或盘多时,将那个盘的数据信息汇报上去
                                                            //或接收到需汇报的命令则从某个时间点开始汇报

    DN2MDS_HEARTBEAT_MESSAGE = 4003,                        //心跳包，带有空间使用情况和多少路在读写

    /**
     *  运维向MDS发送请求，MDS响应
     */
    WEB2MDS_GET_SYSTEM_INFO_MESSAGE = 5001,                 //获取EFS系统信息

    WEB2MDS_GET_MDS_INFO_MESSAGE = 5002,                    //获取MDS信息

    //WEB2MDS_GET_DNS_INFO_MESSAGE = 5003,                  //获取所有或单个DATANODE信息

    WEB2MDS_SYSTEM_CONTROL_INFO_MESSAGE = 5004,             //系统控制信息

    WEB2MDS_SET_CLUSTER_NETWORK_MESSAGE = 5005,             //设置集群网络

    WEB2MDS_GET_DATANODE_INFO_MESSAGE = 5006,               //运维获取集群datanode的信息

    WEB2MDS_ADD_DATANODE_MESSAGE = 5007,                    //运维允许datanode注册

    WEB2MDS_DEL_DATANODE_MESSAGE = 5008,                    //运维禁止datanode注册

    WEB2MDS_EFS_FILE_INFO_MESSAGE = 5009,                   //运维获取EFS文件系统统计信息

    WEB2MDS_BLOCK_RESTORE_MESSAGE = 5010,                   //运维向MDS发起块恢复指令

    WEB2MDS_GET_LICENSE_INFO_MESSAGE = 5011,                //运维向MDS获取license信息

    WEB2MDS_GET_MDS_IP_INFO_MESSAGE = 5012,                 //运维向MDS获取mds ip信息

    WEB2MDS_CONTROL_FILE_RECOVERY_MODE_MESSAGE = 5013,      //运维控制EFS文件恢复模式
	
    WEB2MDS_SET_DNIPTABLE_MESSAGE = 5014,  					//运维向MDS发送网络映射信息   
	
	WEB2MDS_HEARTBEAT_MESSAGE = 5015,                   // 到MDS运维会话的心跳信息
	
    /**
     *  运维向Datanode 发送请求、Datanode响应 相关的消息
     */
    WEB2DN_GET_DATANODE_DETAIL_MESSAGE = 5101,              //获取datanode详细信息

    WEB2DN_GET_DISK_STATUS_MESSAGE = 5102,                  //获取磁盘状态

    WEB2DN_EXCEPTION_DISK_MANAGE_MESSAGE = 5103,            // 异常盘管理协议

    /*
        运维向CatalogServer发送请求，CatalogServer响应
    */
    WEB2CS_SET_CLUSTER_NETWORK_MESSAGE = 5201,              //设置catalog网络配置

    WEB2CS_GET_CLUSTER_NETWORK_MESSAGE = 5202,              //获取catalog网络配置

    WEB2CS_GET_CATALOG_INFO_MESSAGE = 5203,                 //获取catalog状态统计信息

    //注意：这两条协议不应该是5204，5205，需要变为5105，5106 后续变更
	//5204、5205既成事实，不做修改
    WEB2MDS_GET_SAFEMODO_INFO_MESSAGE = 5204,				//获取mds的安全模式

    WEB2MDS_GET_LEAVESAFEMODO_INFO_MESSAGE = 5205,			//获取mds的退出安全模式

    WEB2CS_CREATE_USER_MESSAGE = 5206,						//创建存储用户

    WEB2CS_DELETE_USER_MESSAGE = 5207,						//删除存储用户

    WEB2CS_LIST_USER_MESSAGE = 5208,						//获取用户列表

    WEB2CS_GET_USER_INFO_MESSAGE = 5209,					//获取用户属性

    WEB2CS_UPDATE_USER_INFO_MESSAGE = 5210,					//更新用户信息

    WEB2CS_UPDATE_USER_PASSWORD_MESSAGE = 5211,				//更新用户密码

    WEB2COMMON_HEALTH_CHECK_MESSAGE = 5212,                 //healthchecker检测EFS各个服务是否阻塞协议

    WEB2CS_LIST_RECYCLE_BUCKET_MESSAGE = 5213,              //根据用户id获取该用户下使能了循环覆盖的bucket

    WEB2CS_LIST_RECYCLE_BUCKET_FILE_MESSAGE = 5214,         //批量获取bucketID下的过期文件
	
    WEB2CS_HEARTBEAT_MESSAGE = 5215,                        //生命周期检测服务到CS运维端口的心跳协议

    /* LS主备通信 */
    LSHA_GET_REGION_INFORMATION_MESSAGE = 6101,				//主向备获取Region信息 

    LSHA_PUT_REGION_MESSAGE             = 6102,				//主备实时同步region信息 
    LSHA_PUT_REGION_FILE_MESSAGE        = 6103, 			//主备同步region文件 
    LSHA_HEARTBEAT_DETECT_MESSAGE       = 6104,             //主备通过心跳线进行检测是否存活
    
    /* SDK - LS 通信 */
    OC2LS_REQUEST_LOGIN_MESSAGE         = 6201,				//SDK登陆 LS
    OC2LS_REQUEST_REGIONS_MESSAGE       = 6202,				//SDK向LS获取region信息

    /*
    	DataAnalysis中使用的协议，协议码从[9000,10000)
    */
    //工作者管理者协议
    WK2MG_HEARTBEAT_MESSAGE             = 9000,				//工作者与管理者之间的心跳
    MG2WK_DISPATCH_TASK_MESSAGE         = 9001,				//管理者向工作者下发任务

	/*
		[10000-11000) 保留协议段，用于视频云存储项目，请勿在云存储中使用这一段协议
	*/

    /*
        SDK-Shell - UpgradeSrv 通信，更新SDK库文件时使用
    */
    SS2US_CHECK_SDKLIB_VERSION_MESSAGE  = 12000,			//检测是否为最新版本
    SS2US_DOWNLOAD_SDKLIB_MESSAGE       = 12001				//下载升级包
};

//通知类协议,
enum NotifyType{
    /*
        客户端向MDS发送通知
    */
    OC2MDS_REPORT_BAD_BLOCK_NOTIFY = 6001,                  //报告块客户端发现的错误的信息
                                                            //read的时候发现checksum不正确
                                                            //整体网络不通,需抛弃一组块
                                                            //部分写失败截断汇报

    OC2MDS_RENEW_LEASE_NOTIFY = 6002,                       //更新租约,类似心跳

    /*
        DATANODE向MDS发送通知
    */
    DN2MDS_BLOCK_RECEIVED_NOTIFY = 7001,                    //刚接收完整的块或已关闭的块，汇报给MDS
    DN2MDS_REPORT_BAD_BLOCK_NOTIFY = 7002,                  //和client的bad report 差不多
    DN2MDS_DISK_BROKEN_NOTIFY = 7003,                       //将异常磁盘个数汇报给mds
    DN2MDS_COMMIT_BLOCK_SYNC_NOTIFY = 7004,                 //datanode向mds提交块恢复结果
    DN2MDS_BLOCK_TRANSFER_NOTIFY = 7005,                    //块迁移成功，汇报给MDS
    //DN_ERROR_REPORT_NOTIFY,                               //带错误类型，和错误描述，先不用,保留着

    /**
    *   MDS-HA
    */
    MDSHA_EDITLOG_NOTIFY = 8001,                           //主mds向备mds推送editlog

    MDSHA_MULTICAST_HEARTBEAT_NOTIFY = 8002                //主mds发送主播
    
};

} // namespace CloudStorage
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_PROTOCOL_H__

