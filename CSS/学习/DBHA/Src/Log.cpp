#include <cstdarg>
#include "Log.h"

namespace Dahua {
namespace VideoCloud {

using namespace EFS;

CLog* vcsLogInstance()
{
    static CLog single;
    return &single;	
}

void setVCSPrintLogLevel(int level)
{
    vcsLogInstance()->setPrintLogLevel(level);
}

#define VCS_LOG_MACRO_VA(name, logLevel) \
    int name(const char* fmt, ...) \
{ \
    va_list args; \
    va_start(args, fmt); \
    int ret = vcsLogInstance()->logging( logLevel, false, true, fmt, args ); \
    va_end(args); \
    return ret; \
} 

VCS_LOG_MACRO_VA(vcsLogDebug, CLog::logLevelDebug)
VCS_LOG_MACRO_VA(vcsLogTrace, CLog::logLevelTrace)
VCS_LOG_MACRO_VA(vcsLogInfo, CLog::logLevelInfo)
VCS_LOG_MACRO_VA(vcsLogWarn, CLog::logLevelWarn)
VCS_LOG_MACRO_VA(vcsLogError, CLog::logLevelError)
VCS_LOG_MACRO_VA(vcsLogFatal, CLog::logLevelFatal)

CLog* electLogInstance()
{
	static CLog single;
	return &single; 
}

int electLogInfo( const char* fmt, ... )
{
	va_list args; 
    va_start(args, fmt); 
    int ret = electLogInstance()->logging( CLog::logLevelInfo, false, true, fmt, args ); 
    va_end(args); 
    return ret; 
}


} //VideoCloud
} //Dahua
