//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_RESOURCE_STATISTICS_H__
#define __INCLUDE_DAHUA_EFS_RESOURCE_STATISTICS_H__

#include "Infra/IntTypes.h"
#include <vector>

namespace Dahua{
namespace EFS{

class CResourceStatistics
{	
public:
	CResourceStatistics();
	~CResourceStatistics();
	/**
	*获取主机总cpu负载，返回值cpu% * 10000;
	*eg:
	*cpuLoad = 0.22; so return value is 0.22 * 10000
	*/
	static int getCpuLoad();
	/**
	*获取主机单核子cpu负载，返回值cpu% * 10000;
	*返回vector.size() is cpu核数 +1 ,首个是总load
	*/
	static std::vector<int>  getAllCpusLoad();
	/**
	*获取主机内存使用率，返回值mem% * 10000;
	*/
	static int getMemoryLoad();

	/**
	*获取cpu信息，返回cpu个数和单个cpu的主频
	*注意：cpu个数不一定是实际物理cpu核数(如单核多线程)
	*/
	static void getCpuInfo(uint32_t* cpuNum, uint32_t* perCpuMHz);
	static void getMemoryInfo(int64_t* totalMem, int64_t* freeMem);
	static int getMutiple();
	static std::string getHostName();
	static std::string getUserName();
};

} //namespace EFS
} //namespace Dahua
#endif
