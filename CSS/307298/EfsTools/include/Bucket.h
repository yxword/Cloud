//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_BUCKET_H__
#define __INCLUDE_DAHUA_EFS_BUCKET_H__

#include "Defs.h"
#include "File.h"

namespace Dahua{
namespace EFS{

class CSDKBucketInternal;

class EFS_SDK_API CBucket
{
public:
	CBucket();
	CBucket(const CBucket& rhs);
	CBucket& operator=(const CBucket& rhs);
	~CBucket();
public:
	/**
	 * 获取bucket名
	 * @param buf 接收缓冲区
	 * @param len buf长度，以字节为单位
	 * @return true成功
	 */
	bool getName( char* buf, uint32_t len );
	
	/**
	 * 判断bucket对象是否有效
	 * @return true有效
	 */
	bool isValid();
	
	/**
	 * 设置bucket最大容量
	 * @param megaBytes 容量值，以MB计
	 * @return true成功
	 * 		   false失败，失败原因通过getLastError()获取
	 */
	bool setCapacity( uint64_t megaBytes );

	/**
	 * 获取bucket最大容量
	 * @param [out] megaBytes 容量值，以MB计
	 * @param [out] usedMegaBytes 已使用容量值，以MB计
	 * @return true成功
	 * 		   false失败，失败原因通过getLastError()获取
	 */
	bool getCapacity( uint64_t& megaBytes, uint64_t& usedMegaBytes );
	
	/**
	 * 设置bucket权限
	 * @param	[in]privilege		访问权限，枚举值
	 * @return	true				成功
	 * 			false				失败，失败原因通过getLastError()获取
	 */
	bool setPrivilege(AccessPrivilege privilege);

	/**
	 * 获取bucket权限
	 * @param	[out]privilege		访问权限，枚举值
	 * @return	true				成功
	 * 			false				失败，失败原因通过getLastError()获取
	 */
	bool getPrivilege(AccessPrivilege& privilege);
	
	/**
	 * 设置bucket生命周期
	 * bucket中的文件过期时，执行参数RecycleAction act所指定的操作
	 * 过期条件:当前时间 - 文件创建时间 >= 参数 delay
	 * @param	delay bucket过期时间，单位为秒，最小值为1小时即3600秒
	 * @param	enableAction 是否使能参数act所指定的操作
	 * @param	removeWhenBucketEmpty 当bucket为空时，是否删除
	 * @param	act bucket过期时执行的操作，暂仅支持删除操作
	 * @param	prefix 保留字段
	 * @return	true 设置成功
				false 设置失败，失败原因通过getLastError()获取
	 */
	bool setLifeCycle( int64_t delay, bool enableAction, bool removeWhenBucketEmpty,
					RecycleAction act=doDelete, const char* prefix=NULL); 

	/**
	 * 获取bucket生命周期配置
	 * @param	delay bucket过期时间，单位为秒
	 * @param	enableAction 是否使能act所指定的操作
	 * @param	removeWhenBucketEmpty 当bucket为空时，是否删除
	 * @param	act bucket过期时执行的操作，暂仅支持删除操作
	 * @param	prefix 保留字段
	 * @return	true 获取成功
				false 获取失败，失败原因通过getLastError()获取
	 */
	bool getLifeCycle ( int64_t& delay, bool& enableAction, bool& removeWhenBucketEmpty ,
					RecycleAction& act, const char* prefix = NULL);

	/**
	 * 列出bucket内文件名落在(begin,end]范围的文件名列表
	 * @param begin 起始文件名，不包含begin文件，不包含bucket名
	 * @param end 终止文件名，不包含bucket名
	 * @param maxNumber 最大数量
	 * @param list 输出文件名列
	 * @return true成功
	 * 		   false失败，失败原因通过getLastError()获取
	 */
	bool list( const char* begin, const char* end, uint32_t maxNumber, CNameList& list );
	
	/**
	 * 列出bucket内文件名落在(begin,end]范围的文件名列表
	 * @param begin 起始文件名，不包含begin文件，不包含bucket名
	 * @param end 终止文件名，不包含bucket名
	 * @param maxNumber 最大数量
	 * @param list 输出文件名列
	 * @return true成功
	 * 		   false失败，失败原因通过getLastError()获取
	 */
	bool list( const char* begin, const char* end, uint32_t maxNumber, CFileList& list );
	
	/**
	 * 列出bucket内文件名为dir前缀，以delimiter为后缀的文件名列表
	 * @param dir 文件名前缀，例如 a/b/
	 * @param delimiter 后缀，目前只能是 ‘/’
	 * @param maxNumber 最大数量，例如10
	 * @param list 输出文件名列，例如存在a/b/c a/b/d/ a/b/d/e,得到的结果为‘a/b/c’和‘a/b/d/’
	 * @return true成功
	 * 		   false失败，失败原因通过getLastError()获取
	 */
	bool list(const char* dir, const char* delimiter, 
			const char* begin, uint32_t maxNumber, CFileList& list);
	
	/**
	 * @brief 获取bucket下的文件个数
	 * @pamra bigFileNum 大文件个数
	 * @param smallFileNum 小文件个数
	 * @return ture成功，false失败，失败原因通过getLastError()获取
	 */
	bool getFileNum( uint64_t& bigFileNum, uint64_t& smallFileNum );
	
private:
	friend class CEFileSystem;
	CSDKBucketInternal* m_internal;
};

}
}

#endif

