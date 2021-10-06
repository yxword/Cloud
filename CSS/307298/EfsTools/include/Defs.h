//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_DEFS_H__
#define __INCLUDE_DAHUA_EFS_DEFS_H__

#include "IntTypes.h"
#include <string.h>

// WIN32 Dynamic Link Library
#ifdef _MSC_VER

#ifdef EFS_SDK_DLL_BUILD
#define  EFS_SDK_API _declspec(dllexport)
#elif defined EFS_SDK_DLL_USE
#define  EFS_SDK_API _declspec(dllimport)
#else
#define EFS_SDK_API
#endif

#else

#define EFS_SDK_API

#endif

namespace Dahua{
namespace EFS{

EFS_SDK_API int32_t getLastError();
EFS_SDK_API const char* getErrorMsg( int32_t code );

class EFS_SDK_API CToken
{
public:
	CToken();
	~CToken();
	CToken( const CToken& rhs );
	CToken& operator=( const CToken& rhs );
private:
	class CTokenInternal* m_internal;
};


//初始化配置信息
typedef struct Config
{
	const char* address;				///<元数据IP地址
	uint32_t	port;					///<端口
	const char* userName;
	const char* password;
	char		reserved[120];			///<保留
	Config()
	{
		memset(this, 0, sizeof(Config));
	}
}Config;

//系统信息
typedef struct SystemInfo
{
	uint32_t	totalNodes;				///<总共集群节点数
	uint32_t	totalClient;			///<当前运行的总客户端数
	uint32_t	curWriteFiles;			///<当前正在写的文件数
	uint64_t	totalFiles;				///<总共存储文件数
	uint64_t	totalSpace;				///<总共存储空间量，单位MB
	uint64_t	spaceLeft;				///<剩余存储空间量，表示系统真实可写剩余量，单位MB
    char        uuid[32];               ///<系统uuid,32个字符
	uint64_t    spaceQuotaLeft;         ///<剩余可用空间，单位MB
    uint64_t    spaceUsed;              ///<用户已使用容量, 单位MB
    char		reserved[44];			///<保留
}SystemInfo;

//文件属性
typedef struct FileStat
{
	uint8_t		mode;					///<读写模式
	uint8_t		isAppend;				///<是否正在被写入
	uint8_t		dataNum;				///<数据分片数	
	uint8_t		parityNum;				///<冗余分片数	
	uint32_t	cTime;					///<创建时间
	uint32_t	mTime;					///<最后修改时间	
	uint64_t	totalSize;				///<文件大小
	char		reserved2[108];
}FileStat;

//文件读写模式
enum FileMode
{
	fileModeRead = 1, 					///<读
	fileModeWrite,     					///<写
	fileModeAppend						///<大文件追加写
};

//文件指针位置
enum FileLocation
{
	efsBegin,      						///<开始位置
	efsCurrent,     					///<当前位置
	efsEnd								///<文件尾
};

enum EFSOption
{
	efsConcurrent,  					///<并发数
	efsBufferSize,  					///<缓存大小
	efsTimeOut,      					///<超时时间
	efsLogOutput,						///<日志文件输出路径
	efsLogLevel,						///<日志级别
	efsRwMode,							///<读写模式(0异步1同步)
	efsSmallFileBufferSize,				///<小文件缓存大小,CEFileSystem.setOption设置影响后续构造的小文件buffer
	efsBufferMode						///<读缓存模式(1默认单缓存2双缓存用于提高频繁seek场景下的读性能)
};

//日志级别
enum{
	FATALF=1,
	ERRORF,
	WARNF,
	INFOF,
	TRACEF,
	DEBUGF
};

enum EFSState {
	efsStateInit,						///初始状态
	efsStateNormal,						///<efs客户端正常
	efsStateException,					///<efs客户端异常，会进行自动恢复
	efsStateError          				///<efs客户端异常，可能是用户不存在或者密码错误，不可自动恢复，需要用户介入.
};

enum FileState {
	fileStateInit,						///初始状态
	fileStateNormal,					///<文件正常
	fileStateWarning,					///<文件有风险
	fileStateException					///<文件异常
};

enum AccessPrivilege {
	privilegePrivate	= 0,			///<私有，其他用户不可见
	privilegeRead		= 1,			///<只读，其他用户无创建、删除、写入权限
	privilegeWrite		= 2				///<读写，其他用户有创建、删除、写入权限
};

//bucket循环覆盖操作类型
enum RecycleAction{
	doDelete = 0						///<删除
};

//使用空间达到阈值时的回收策略
enum RecyclePolicy{
	reduceByTime = 0,					///<等时间缩减
	reduceByRatio = 1,					///<等比例缩减
};

class EFS_SDK_API CRedundanceCaps
{
public:
	CRedundanceCaps();
	~CRedundanceCaps();
	int size() const;						///<总共支持的组合类型
	uint16_t getDataNum(int index) const;	///<N
	uint16_t getParityNum(int index) const;	///<M
private:
	CRedundanceCaps(const CRedundanceCaps& rhs);
	CRedundanceCaps& operator=(const CRedundanceCaps& rhs);
	class CRedundanceCapsInternal *m_internal;
};

class CNameListInternal;
class EFS_SDK_API CNameList
{
public:
	CNameList();
	CNameList(const CNameList& rhs);
	CNameList& operator = (const CNameList& rhs);
	~CNameList();
	int size() const;
	const char* getName(int index) const;
private:
	CNameListInternal* m_internal;
};

class FileListInternal;
class EFS_SDK_API CFileList
{
public:
	CFileList();
	CFileList(const CFileList& rhs);
	CFileList& operator=(const CFileList& rhs);
	~CFileList();
	int size() const;
	const char* getName(int index) const;
	uint64_t getFileSize(int index) const;
	uint32_t getCTime(int index) const;
	uint32_t getMTime(int index) const;
	const char* getFileAttribute(int index) const;
	uint32_t getFileAttrLength(int index) const;
private:
	FileListInternal* m_internal;
};

class CEFileSystem;
class EFS_SDK_API IEFSListener {
public:
	virtual ~IEFSListener() {}

	/**
	 * efs状态变化回调
	 * @param	efs			状态变化对应的efs
	 * @param	state		efs新状态
	 */
	virtual void onEFSStateChanged(CEFileSystem* efs, EFSState state) = 0;
};

class CFile;
class EFS_SDK_API IFileListener {
public:
	virtual ~IFileListener() {}

	/**
	 * 文件状态变化回调
	 * @param	file		状态变化对应的文件
	 * @param	state		file新状态
	 */
	virtual void onFileStateChanged(CFile* file, FileState state) = 0;
};

	/**
	 * 禁用backtrace功能, 此接口必须先与其他接口调用。
	 * @return  true    成功
	 *          false   失败，如果backtrace功能已经启用，会返回失败。
	 */
bool disableBacktrace();

}
}

#endif //__INCLUDE_DAHUA_EFS_DEFS_H__
