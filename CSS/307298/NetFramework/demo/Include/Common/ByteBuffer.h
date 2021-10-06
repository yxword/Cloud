//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_BUFFER_H__
#define __INCLUDE_DAHUA_EFS_BUFFER_H__

#include "Infra/IntTypes.h"

namespace Dahua{
namespace EFS{

class CByteBuffer
{
public:
	//size 分配空间的长度
	//autoMove 自动调整buffer空间
	explicit CByteBuffer(const uint32_t size,bool autoMove=true);
	//新增构造，可以指定内部分配的内存按alignment长度对齐
	explicit CByteBuffer(const uint32_t size,uint32_t alignment,bool autoMove=false);
	virtual ~CByteBuffer();

	//消费了多少数据,对应于getData,取得数据数据指针,消费了多少数据
	bool drain(const uint32_t len);
	//生产了多少数据,对应于getFree,从空闲区域写入多少数据
	bool pour(const uint32_t len);
	
	//获取空闲指针
	char* getFree();

	//获取空闲长度
	uint32_t getFreeLength();

	//获取数据指针
	char* getData();

	//获取数据长度
	uint32_t getDataLength();

	//获取bytebuffer的空间容量
	uint32_t capacity();

	//重置数据指针,空闲指针到起始位置
	void reset();

	//空闲长度过小时,可以反转,将已有数据移动到起始位置
	bool compact();

private:
	struct Internal;
	struct Internal*	m_internal;
};

} //namespace EFS
} //namespace Dahua
#endif //__INCLUDE_DAHUA_EFS_BUFFER_H__

