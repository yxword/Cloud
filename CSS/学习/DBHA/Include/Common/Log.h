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
    enum LogLevel
    {
        logLevelUnknown= 0, ///< 未知打印等级类型.
        logLevelFatal,	///< fatal等级，当设置为此等级时，有一种打印输出（fatal）都有输出.
        logLevelError,	///< error等级，当设置为此等级时，有两种打印输出（fatal，error）都有输出.
        logLevelWarn,	///< warn等级，当设置为此等级时，有三种打印输出（fatal，error，warn）都有输出.
        logLevelInfo,	///< info等级，当设置为此等级时，有四种打印输出（fatal，error，warn，info）都有输出.
        logLevelTrace,	///< Trace等级，当设置为此等级时，有五种打印输出（fatal，error，warn，info，trace）都有输出.
        logLevelDebug,	///< Debug等级，当设置为此等级时，以上六种打印（fatal，error，warn，info，trace，debug）都有输出.
    };

    //保持兼容, 后续请使用loglevel
    enum
    {
        FATALF=1,
        ERRORF,
        WARNF,
        INFOF,
        TRACEF,
        DEBUGF
    };
public:
	CLog();
    virtual ~CLog();

	/**
	 * @func:
	 * 构造单例句柄
	 */
	static CLog* instance();
	/*
	 * @param:
	 * 1 strLogPath 输出日志路径，自动mkdir.
	 * 2 strLogFilePrefix 日志文件模块名.
	 * 3 nLogLevel 日志等级 适配Infra日志等级，default 6 debug.
	 * 4 attachStdout 是否同时打印到标准输出，default false.
	 * 5 splitBySize 是否按大小切割,true表示按大小切割，false表示按天切割.
	 * 6 nSplit 日志按多大切割,单位MB，如果按天切割，该参数会被忽略.
	 * 7 nRecycle 日志回滚的个数，0表示表示不回滚.
	 * 8 maxBufSize 日志缓存大小.
	 * 9 attachInfraLog 是否使用Infra的日志.
	 */
	void init(std::string strLogPath="./", std::string strLogFilePrefix = "", 
				int nLogLevel = logLevelDebug, bool attachStdout= false, bool splitBySize = true,
				int nSplit=30, int nRecycle = 5, int maxBufSize = 512*1024, bool attachInfraLog = true);

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

	/**
	 * @func:
	 * 设置不缓存
	 */
	void setNoCache(bool bNoCache = true);

    void syncLog();

    /**
     * logging函数
     */
    int logging(int level, bool infraLog, bool logTimestamp, const char* fmt, ... ) __attribute__((format(printf, 5, 6)));	
    int logging(int level, bool infraLog, bool logTimestamp, const char* fmt, va_list args );	

    int logDebug(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    int logTrace(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    int logInfo(const char* fmt, ...)  __attribute__((format(printf, 2, 3)));
    int logWarn(const char* fmt, ...)  __attribute__((format(printf, 2, 3)));
    int logError(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    int logFatal(const char* fmt, ...) __attribute__((format(printf, 2, 3)));

private:
	CLog(const CLog& log);
	CLog& operator=(const CLog& log);
private:
	class Internal;
	Internal* m_internal;
};

// 以下接口的调用，日志会输出到唯一的CLog::instance()对应的日志实例中
// 如果需要独立输出到另外的日志文件，则需要自行定义类似的以下接口，指向自行实例化的CLog对象中
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
