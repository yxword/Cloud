//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_FILE_SYSTEM_H__
#define __INCLUDE_DAHUA_EFS_FILE_SYSTEM_H__

#include "Defs.h"
#include "File.h"
#include "Bucket.h"
#include <stdlib.h>

namespace Dahua{
namespace EFS{

class CEFSInternal;

class EFS_SDK_API CEFileSystem
{
private:
	CEFileSystem( const CEFileSystem & rhs );
	CEFileSystem& operator=(const CEFileSystem& rhs);
public:
	CEFileSystem();
	~CEFileSystem();

	/**
	 * 初始化文件系统
	 * @param  cfg 配置文件
	 * @return ture成功
	 * 	   	   false失败，失败原因通过getLastError()获取
	 *	   	   可能的error code有：
	 * 	   	   efsOK 成功
	 *         efsNetError 网络异常
	 *         efsUserInexistent 用户不存在
	 *         efsErrorPassword 密码错误
	 *         efsMaxClientLimit 达到最大客户端数量
	 *         efsParamError 参数错误
	 */
	bool initialize( const Config& cfg );
	
	/**
	 * 设置文件系统监听者
	 * @param	listener		文件系统监听者。为NULL时表示删除监听
	 * @return	true			成功
	 *			false			失败，失败原因通过getLastError()获取
	 * 可能的error code有：
	 *			efsOK			成功
	 *			efsFail			失败
	 */
	bool setListener(IEFSListener* listener);

	/**
	 * 设置系统高级配置
	 * @param  key 	 配置项，可选值为：
	 * 				 efsConcurrent  文件并发数
	 *				 efsBufferSize  文件默认缓存大小
	 *				 efsTimeOut     调用超时时间
	 *				 efsLogOutput	日志输出路径
	 *				 efsLogLevel    日志级别(1:fatal 2:error 3:warn 4:info 5:tarce 6:debug)
	 *				 efsRwMode      读写模式(0异步1同步)
	 *				 efsSmallFileBufferSize   小文件缓存大小
	 *				 efsBufferMode  读缓存模式(1默认单缓存2双缓存用于提高频繁seek场景下的读性能)
	 * @param  value 配置项key对应的值
	 * @param  len   value长度，以字节为单位
	 * @return ture成功
	 *		   false失败，失败原因通过getLastError()获取
	 *		   可能的error code有：
	 *		   efsOK 成功
	 *		   efsParamError 参数错误
	 *		   efsUnsupported 不支持
	 */
	bool setOption(EFSOption key, const void* value, uint32_t len);
	
	/**
	 * 获取系统高级配置
	 * @param  key   配置项，可选值为：
	 * 				 efsConcurrent  文件并发数
	 *				 efsBufferSize  文件默认缓存大小
	 *				 efsTimeOut     调用超时时间
	 *				 efsLogOutput	日志输出路径
	 *				 efsLogLevel    日志级别(1:fatal 2:error 3:warn 4:info 5:tarce 6:debug)
	 *				 efsRwMode      读写模式(0异步1同步)
	 *				 efsSmallFileBufferSize   小文件缓存大小
	 *				 efsBufferMode  读缓存模式(1默认单缓存2双缓存用于提高频繁seek场景下的读性能)
	 * @param  value 配置项key对应的值
	 * @param  len   value长度，以字节为单位
	 * @return ture成功
	 *		   alse失败，失败原因通过getLastError()获取
	 *		   可能的error code有：
	 *		   efsOK 成功
	 *		   efsParamError 参数错误
	 *		   efsUnsupported 不支持
	 */
	bool getOption(EFSOption key, void* value, uint32_t len);
	
	/**
	 * 获取系统信息
	 * @param  info 系统信息结构体
	 * @return true成功
	 *		   false失败，失败原因通过getLastError()获取
	 */
	bool getSystemInfo( SystemInfo& info );
	
	/**
	 * 删除文件
	 * @param  fileName 文件名
	 * @return true成功
	 *		   false失败，失败原因通过getLastError()获取
	 *		   可能的error code有：
	 *		   efsOK 成功
	 *		   efsFail 失败
	 *		   efsParamError 参数错误
	 */
	bool remove( const char* fileName );	

	/**
	 * 创建文件
	 * @param  fileName 文件名
	 * @param  n 冗余参数，和参数m一起表示n+m，其中n表示n个数据分片，m表示m个数据分片，
	 *         n+m表示n个数据分片形成m个冗余，支持m个错误发生而不影响数据的完整性
	 * @param  m 冗余参数
	 * @param  bigFile 是否大文件
	 * @return 文件实例，通过调用CFile::isValid()来判断创建是否成功
	 *		   若失败，通过getLastError()获取失败原因
	 */
	CFile create( const char* fileName, uint16_t n = 4, uint16_t m = 1, bool bigFile = true );

	/**
	 * 重命名文件，只对Bucket内文件有效
	 * @param  oldName 原文件名
	 * @param  newName 新文件名
	 * @return true成功
	 *		   false失败，失败原因通过getLastError()获取
	 *		   可能的error code有：
	 *		   efsOK 成功
	 *		   efsFail 失败
	 *		   efsParamError 参数错误
	 */
	bool rename( const char* oldFileName, const char* newFileName );

	/**
	 * 打开文件
	 * @param  fileName 文件名
	 * @param  mode 文件模式，可选值为：
	 *		   fileModeRead  读模式
	 *         fileModeWrite 写模式（暂不支持）
	 * @return CFile实例，通过调用CFile::isValid()来判断创建是否成功
	 *		   若失败，通过getLastError()获取失败原因
	 */
	CFile open( const char* fileName, FileMode mode );
	
	/**
	 * 关闭文件系统
	 * @return true成功
	 *		   false失败，失败原因通过getLastError()获取
	 *		   可能的error code有：
	 *		   efsOK 成功
	 *		   efsFail 失败
	 */
	bool close();
	
	/**
	 * 锁定文件
	 * @param	fileName 文件名
	 * @return	true成功 
	 *			false失败，失败原因通过getLastError()获取
	 */
	bool lockFile( const char* fileName);
	
	/**
	 * 解锁文件
	 * @param	fileName 文件名
	 * @return	true成功 
	 *			false失败，失败原因通过getLastError()获取
	 */
	bool unlockFile( const char* fileName);

	/**
	 * 获取文件锁定状态
	 * @param	fileName 文件名,包含bucket名
	 * @return	1 锁定
	 *			0 未锁定
	 *			-1 获取失败，失败原因通过getLastError()获取
	 */
	int32_t getFileLockStat(const char* fileName);

	/** 
	 * 创建一个bucket
	 * @param name bucket名字，须大于等于1字节，小于等于63字节
	 * @return CBucket实例
	 */
	CBucket createBucket( const char* name );

	/**
	 * 删除一个bucket
	 * @param  name bucket名字，大于等于1字节，小于等于63字节
	 * @return true成功
	 *		   false失败，失败原因通过getLastError()获取
	 *		   可能的error code有：
	 *		   efsOK 成功
	 *		   efsFail 失败
	 *		   efsParamError 参数错误
	 */
	bool removeBucket( const char* name );

	/**
	 * 获取一个bucket
	 * @param name bucket名字
	 * @return CBucket实例，通过调用CBucket::isValid()来判断获取是否成功
	 *		   若失败，通过getLastError()获取失败原因
	 */
	CBucket getBucket( const char* name );

	/**
	 * 获取bucket列表
	 * @param  token 查询令牌，需用户自定义
	 * @param  maxNumber 最大数量
	 * @param  list 返回的bucket名列表
	 * @return true成功
	 *		   false失败，失败原因通过getLastError()获取
	 *		   可能的error code有：
	 *		   efsOK 成功
	 *		   efsFail 失败
	 */
	bool listBucket(CToken& token, uint32_t maxNumber, CNameList& list);
	/**
	 * 获取EFS支持的N+M冗余类型
	 */
	bool getRedundanceCaps(CRedundanceCaps& caps);

	/**
	 * 设置用户空间回收策略,仅对已使能生命周期的bucket有效
	 * @param	enableRecycle 是否使能用户空间回收
	 * @param	policy: reduceByTime  等时间缩减
						reduceByRatio 等比例缩减
	 * @return	true成功
				false失败，失败原因通过getLastError()获取
	 */
	bool setRecyclePolicy( bool enableRecycle , RecyclePolicy policy);

	/**
	 * 获取用户空间回收策略配置
	 * @param	enableRecycle 是否使能用户空间回收
	 * @param	policy: reduceByTime  等时间缩减
						reduceByRatio 等比例缩减
	 * @return	true成功
				false失败，失败原因通过getLastError()获取
	 */
	bool getRecyclePolicy( bool& enableRecycle, RecyclePolicy& policy);
private:
	CEFSInternal* m_internal;
};

}
}

#endif
