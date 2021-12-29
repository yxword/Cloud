//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_FILENAME_H__
#define __INCLUDE_DAHUA_EFS_FILENAME_H__

#include "Infra/IntTypes.h"

namespace Dahua {
namespace EFS {

typedef struct
{
	uint64_t 	fileId;		//文件ID
	uint8_t		fileType;	//文件类型
	uint8_t		clusterId;	//集群号
	uint8_t		familyIndex;	//族索引号
	uint8_t		reserved;		//保留
	char		suffix[8];	//后缀名
}FileName;

class CFileName
{
public:
	CFileName();
	//打印字符文件名构造
	CFileName( const char* name );
	//文件ID结构体构造
	CFileName( FileName& name );
	~CFileName();
	CFileName& operator=(const CFileName& other);
	//获取文件名,name长度必须大于等于24
	bool getName( char* name );
	//获取文件后缀，suffix长度必须大于等于8
	bool getSuffix( char* suffix );
	//获取结构体形式文件名
	bool getName( FileName& name );
private:
	struct Internal;
	struct Internal*	m_internal;
};


} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_FILENAME_H__
