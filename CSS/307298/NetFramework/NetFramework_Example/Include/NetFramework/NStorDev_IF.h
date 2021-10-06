//
//  "$Id: NStorDev_IF.h 7672 2010-5-10 02:28:18Z zhou_mingwei $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  	Modification
//					2011-3-30		zhou_mingwei	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_NSTORDEV_IF_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_NSTORDEV_IF_H__

#include "Infra/IntTypes.h"
#include "NDefs.h"

namespace Dahua{
namespace NetFramework{

struct NFile_Info
{	
	uint64_t	time_create;	///< 文件创建时间
	uint64_t	time_access;	///< 文件访问时间
	uint64_t	time_modify;	///< 文件修改时间
	uint64_t	file_size;		///< 文件大小
	uint32_t	attrib;			///< 文件属性
	char	name[256];		///< 文件名
};  
/// 文件属性
enum{
	NORMAL =    0x00,
	READ_ONLY =  0x01,
	HIDDEN =    0x02,
	SYSTEM =    0x04,
	VOLUME =    0x08,
	DIRECTORY = 0x10,
	ARCHIVE =   0x20
};
enum{
	FILE_HEAD = 0, 	//文件起始
	FILE_CUR,		//当前位置
};

//这个类定义了一个可以被NFile使用的文件设备所必须具备的接口。
//用户继承这个类进行自己的文件系统I/O功能的实现，并加入到NFile中，
//就可以实现基于网络框架的异步磁盘I/O。如果使用标准的Linux文件系统，
//可以不用管这个接接口，直接给NFile传入NULL值即可。
class NETFRAMEWORK_API CNStorDev_IF
{
public:
	CNStorDev_IF(){}
	virtual ~CNStorDev_IF(){}
	//打开一个文件
	//参数：path：文件路径，可以是虚拟的文件路径
	//			mode：打开方式，可以是虚拟的打开方式
	//返回值：-1:失败，0：成功
	virtual int Open( const char* path, const char* mode ) = 0;
	//关闭文件
	//注意：在该接口的实现中，需要delete this来销毁自己，或者类似的
	//			销毁自己（比如引用计数减一），总之，调用完Close，NFile就不会
	//			再管该对象了。所以，所有的CNStorDev_IF对象的创建和销毁必须配对。
	virtual void Close() = 0;
	//写文件
	//参数：	buf：数据缓冲区
	//				len：数据长度
	//返回值：完成写入量，-1失败
	virtual int Write( const char* buf, const uint32_t len ) = 0;
	//读文件
	//参数：	buf：缓冲区
	//				len：读取的最大值
	//返回值：-1:失败,0：读到文件尾，大于0：实际读取到的量
	virtual int Read( char* buf, const uint32_t len ) = 0;	
	//定位文件
	//参数：offset：偏移量
	//			whence：定位起始点
	//返回值：-1：失败，0：成功
	virtual int Seek( int64_t offset, int whence ) = 0;
	//获取文件状态
	//参数：state：保存状态的结构体
	//返回值：-1：失败，0：成功
	virtual int Stat( struct NFile_Info* state ) = 0;
	//该存储设备是否需要异步磁盘I/O功能？如果不需要，NFile将直接调用该设备I/O接口。
	//如果存储设备已经具备的异步磁盘I/O功能，或者作了很好的数据缓冲，就可以重载
	//这个接口，返回flase。缺省返回true，即需要AIO功能。
	virtual bool NeedAIO() { return true; }
};	

}//NetFramework
}//Dahua
	
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_NSTORDEV_IF_H__
