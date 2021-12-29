
//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//					2016-11-10		江文龙		1.0
//					2016-11-22		江文龙		1.1 发布头文件将RowLock.h合并在RowLockManager.h当中

// 用途: 为了解决行级记录的大粒度加锁影响并发性能,实现细粒度的行锁
// 使用场景: 如EFS中的catalogserver在对并行双写处理没有保证行级数据的串行修改需要使用该行锁
// 使用方式:
// CRowLockManger lockmanager
// rowlock = lockmanager.getRowLock(key,len)
// if rowlock == NULL 
//	process getlock timeout
//  return
// fi
//  process something
// rowlock.release()
//


#ifndef WIN32
#ifndef __INCLUDE_DAHUA_CLOUD_EFS_ROW_LOCK_MANAGER_H__
#define __INCLUDE_DAHUA_CLOUD_EFS_ROW_LOCK_MANAGER_H__

#include "Infra/Types.h"
#include "Memory/SharedPtr.h"

namespace Dahua{
namespace EFS{

class CRowLock
{
public:
	CRowLock();
	~CRowLock();

	// 业务方在通过CRowLockManger::getRowLock成功并处理外业务后需要调用该函数进行锁释放
	void release();

private:
	struct Internal;
	struct Internal*	m_internal;
};

class CRowLockManager
{
public:
	//默认请求锁当锁被占用则最多等待10s,单位毫秒。
	CRowLockManager(uint32_t waitMilliSecond=10000); 
	~CRowLockManager();

	// 请求行锁,当锁请求不到超时返回NULL
	Memory::TSharedPtr<CRowLock> getRowLock( const char* key, int len );

private:
	struct Internal;
	struct Internal* 	m_internal;
};

} //end namespace EFS
} //end namespace Dahua

#endif //__INCLUDE_DAHUA_CLOUD_EFS_ROW_LOCK_MANAGER_H__
#endif //WIN32

