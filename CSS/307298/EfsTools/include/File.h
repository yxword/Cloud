//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_FILE_H__
#define __INCLUDE_DAHUA_EFS_FILE_H__

#include "Defs.h"

namespace Dahua{
namespace EFS{

class CEFileSystem;
class CSDKFileInternal;

class EFS_SDK_API CFile
{
public:	
	CFile();
	CFile( const CFile& rhs );
	CFile& operator=(const CFile& rhs);
	~CFile();	
	
	/**
	 * 判断文件是否合法
	 * @return true 合法
	 */
	bool isValid();
	
	/**
	 * 设置文件监听者
	 * @param	listener		文件监听者。为NULL时表示删除监听
	 * @return	true			成功
	 *			false			失败，失败原因通过getLastError()获取
	 * 可能的error code有：
	 *			efsOK			成功
	 *			efsFail			失败
	 */
	bool setListener(IFileListener* listener);

	/**
	 * 设置文件高级配置
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
	 * @return true成功
	 * 		   false失败，失败原因通过getLastError()获取
	 * 		   可能的error code有：
	 * 		   efsOK 成功
	 * 		   efsParamError 参数错误
	 * 		   efsInvalidFile 文件无效
	 * 		   efsUnsupported 不支持
	 */
	bool setOption(EFSOption key, const void* value, uint32_t len);

	/**
	 * 获取文件高级配置
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
	 * @return true成功
	 * 		   false失败，失败原因通过getLastError()获取
	 * 		   可能的error code有：
	 * 		   efsOK 成功
	 *		   efsParamError 参数错误
	 * 		   efsInvalidFile 文件无效
	 * 		   efsUnsupported 不支持
	 */
	bool getOption(EFSOption key, void* value, uint32_t len);

	/**
	 * 设置用户属性
	 * @param  attribute   attribute为用户输入的自定义属性，length为缓冲区长度，不允许超过32字节。
	 * @param  length      attribute长度，规定为32字节以字节为单位，用户需要保证attribute与length的对应关系。
	 * @return true成功
	 * 		   false失败，失败原因通过getLastError()获取
	 */
	bool setAttribute(const char *attribute, uint32_t length);

	/**
	 * 获取用户属性
	 * @param  attribute   attribute为用户输入的自定义属性，length为缓冲区长度，不允许超过32字节。
	 * @param  length      attribute长度，规定为32字节以字节为单位
	 * @return 大于等于0实际获取到的自定义属性长度
	 * 		   小于0失败
	 */
	int32_t getAttribute(char *attribute,uint32_t length);

	/**
	 * 获取文件名
	 * @param  name 存放文件名的buffer
	 * @param  len  buffer长度，必须大于28字节，如果是bucket模式，必须大于1088字节
	 * @return true 成功
	 */
	bool getFileName( char* name, uint32_t len );
	
	/**
	 * 写文件
	 * @param  buf 待写入数据
	 * @param  len 数据长度
	 * @return 0暂时不可写，待重试
	 * 		   大于0实际写入量
	 * 		   -1文件异常，需要关闭文件
	 * 		   失败原因通过getLastError()获取
	 */
	int write( const char* buf, uint32_t len );
	
	/**
	 * 读文件
	 * @param  buf 读出数据buffer
	 * @param  len buf长度
	 * @return     0暂时不可读，待重试
	 * 			   大于0实际读出量
	 * 			   -1文件异常，需要关闭文件
	 * 			   -2读到文件尾		   
	 * 			   失败原因通过getLastError()获取
	 */
	int read( char* buf, uint32_t len );
	
	/**
	 * 文件定位
	 * @param  offset 相对于whence的偏移
	 * @param  whence seek类型，可选值为：
	 * 				  efsBegin文件头
	 * 				  efsCurrent当前文件位置
	 * 				  efsEnd文件尾
	 * @return ture成功
	 * 		   false失败，失败原因通过getLastError()获取
	 * 		   可能的error code有：
	 * 		   efsOK 成功
	 * 		   efsFail 失败
	 * 		   efsInvalidFile 文件无效
	 */
	bool seek( int64_t offset, FileLocation whence );
	
	/**
	 * 获取文件位置
	 * @return 文件当前相对于头的偏移位置
	 * 		   efsInvalidFile 文件无效
	 */
	int64_t tell();

	/**
	 * 关闭文件
	 * @return ture成功
	 * 		   false失败，失败原因通过getLastError()获取
	 * 		   可能的error code有：
	 * 		   efsOK 成功
	 * 		   efsFail 失败
	 * 		   efsInvalidFile 文件无效
	 */
	bool close();

	/**
	 * 获取文件属性
	 * @param  stat 存放文件属性
	 * @return ture成功
	 * 		   false失败，失败原因通过getLastError()获取
	 * 		   可能的error code有：:
	 * 		   efsOK 成功
	 * 		   efsFail 失败
	 * 		   efsInvalidFile 文件无效
	 */
	bool stat( FileStat& stat );
	
	/**
	 * 将缓存内的数据刷入存储
	 * @retval true 成功
	 * @retval false 失败
	 */
	bool flush();
private:
	friend class CEFileSystem;
	CSDKFileInternal*	m_internal;
};

}
}

#endif //__INCLUDE_DAHUA_EFS_FILE_H__
