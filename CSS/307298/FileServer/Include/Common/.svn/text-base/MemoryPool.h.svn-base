
#ifndef __INCLUDE_DAHUA_EFS_MEMORY_POOL_H__
#define __INCLUDE_DAHUA_EFS_MEMORY_POOL_H__

#include "Infra/IntTypes.h"

namespace Dahua {
namespace EFS {

class CMemoryPool
{
public:
    CMemoryPool(bool align = false);//默认不对齐
    virtual ~CMemoryPool();

    void* malloc(uint32_t size);
    void free(void* pMem); 
private:
    class Internal;
    class Internal* m_internal;    

};

} //namespace EFS
} //namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_MEMORY_POOL_H__
