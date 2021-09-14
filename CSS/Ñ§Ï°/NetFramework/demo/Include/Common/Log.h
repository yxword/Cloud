//
//
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef _CLOG_H_
#define _CLOG_H_

#include <stdio.h>
#include <string.h>
#include <string>
#include "Infra/PrintLog.h"

namespace Dahua {
namespace EFS {

class CLog {
public:
	enum{
		FATALF=1,
		ERRORF,
		WARNF,
		INFOF,
		TRACEF,
		DEBUGF
	};
public:
	/**
	 * @func:
	 * 构造单例句柄
	 */
	static CLog* instance();
	/*
	 * @param:
	 * 1 strLogPath 输出日志路径，自动mkdir
	 * 2 strLogFilePrefix 日志文件模块名
	 * 3 nLogLevel 日志等级 适配Infra日志等级，default 6 debug
	 * 4 isStdout 是否同时打印到标准输出，default false
	 * 5 splitBySize 是否按大小切割,true表示按大小切割，false表示按天切割
	 * 6 mSplit 日志按多大切割,单位MB，如果按天切割，该参数会被忽略
	 * 7 nRecycle 日志回滚的个数，0表示表示不回滚
	 */
	void init(std::string strLogPath="./", std::string strLogFilePrefix = "", 
				int nLogLevel = DEBUGF, bool isStdout= false, bool splitBySize = true,
				int mSplit=30, int nRecycle = 5, int cacheBufSize = 512*1024, bool useInfraLog = true);
	/**
	 * @func:
	 * CLog适配器attach到Infra:log
	 *	@return:
	 *	 成功返回0, 失败返回-1
	 */
	int attachPrinter();
	/**
	 * @func:
	 * CLog适配器detach到Infra:log
	 *	@return:
	 *	 成功返回0, 失败返回-1
	 */
	int detachPrinter();
	void attachStdout();
	void detachStdout();
	void setPrintLogLevel(int level);
	virtual ~CLog();

	/**
	 * @func:
	 * 设置不缓存
	 */
	void setNoCache(bool bNoCache = true);
private:
	CLog();
	CLog(const CLog& log);
	CLog& operator=(const CLog& log);
private:
	class Internal;
	Internal* m_internal;
};

void setEfsPrintLogLevel(int level);
int  efsLogDebug(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
int  efsLogTrace(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
int  efsLogInfo(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
int  efsLogWarn(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
int  efsLogError(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
int  efsLogFatal(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

} /* namespace EFS */
} /* namespace Dahua */
#endif /* CLOG_H_ */
