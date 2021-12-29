#ifndef __DAHUA_VIDEOCLOUD_DBHA_HALOG_H_
#define __DAHUA_VIDEOCLOUD_DBHA_HALOG_H_

#include "Log.h"

#define DBHALOG_CLASSNAME( a ) static const char* s_logClassName = #a

#define DBHALOG_FATAL( format, ... ) \
    LOG_FATAL( "#%s::%s %d, "format"\n", s_logClassName, __FUNCTION__,__LINE__, ## __VA_ARGS__ )
#define DBHALOG_ERROR( format, ... ) \
	LOG_ERROR( "#%s::%s %d, "format"\n", s_logClassName, __FUNCTION__,__LINE__, ## __VA_ARGS__ )
#define DBHALOG_WARN( format, ... ) \
	LOG_WARN( "#%s::%s %d, "format"\n", s_logClassName, __FUNCTION__,__LINE__, ## __VA_ARGS__ )
#define DBHALOG_INFO( format, ... ) \
    LOG_INFO( "#%s::%s %d, "format"\n", s_logClassName, __FUNCTION__,__LINE__, ## __VA_ARGS__ )
#define DBHALOG_TRACE( format, ... ) \
    LOG_TRACE( "#%s::%s %d, "format"\n", s_logClassName, __FUNCTION__,__LINE__, ## __VA_ARGS__ )
#define DBHALOG_DEBUG( format, ... ) \
    LOG_DEBUG( "#%s::%s %d, "format"\n", s_logClassName, __FUNCTION__,__LINE__, ## __VA_ARGS__ )


#endif // __DAHUA_VIDEOCLOUD_DBHA_HALOG_H_


