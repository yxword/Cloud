#ifndef __DAHUA_VideoCloud_LOG_H__
#define __DAHUA_VideoCloud_LOG_H__

#include "Infra/PrintLog.h"
#include "Common/Log.h"

//using infra log
#undef LOG_DEBUG 
#undef LOG_TRACE 
#undef LOG_INFO 
#undef LOG_WARN 
#undef LOG_ERROR 
#undef LOG_FATAL 

#define LOG_DEBUG vcsLogDebug
#define LOG_TRACE vcsLogTrace
#define LOG_INFO  vcsLogInfo
#define LOG_WARN  vcsLogWarn
#define LOG_ERROR vcsLogError
#define LOG_FATAL vcsLogFatal

namespace Dahua {
namespace VideoCloud {

Dahua::EFS::CLog* vcsLogInstance();
void setVCSPrintLogLevel(int level);
int  vcsLogDebug(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
int  vcsLogTrace(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
int  vcsLogInfo(const char* fmt, ...)  __attribute__((format(printf, 1, 2)));
int  vcsLogWarn(const char* fmt, ...)  __attribute__((format(printf, 1, 2)));
int  vcsLogError(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
int  vcsLogFatal(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

Dahua::EFS::CLog* electLogInstance();
int electLogInfo( const char* fmt, ...) __attribute__((format(printf, 1, 2)));

} //VideoCloud
} //Dahua

#endif // __DAHUA_VideoCloud_LOG_H__
