
#ifndef WIN32

#ifndef __STOUT_OS_VPOPEN__
#define __STOUT_OS_VPOPEN__

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

namespace Dahua{
namespace EFS{

FILE* vpopen( const char *cmdstring, const char *type );
int vpclose( FILE *fp );

} //end namespace EFS
} //end namespace Dahua

#ifdef __cplusplus
}
#endif

#endif // __STOUT_OS_VPOPEN__

#endif // WIN32


