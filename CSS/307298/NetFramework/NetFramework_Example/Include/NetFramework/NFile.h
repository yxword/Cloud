//
//  "$Id: NFile.h 7672 2010-5-10 02:28:18Z zhou_mingwei $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  	Modification
//					2011-3-30		zhou_mingwei	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_NFILE_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_NFILE_H__

#include "Infra/IntTypes.h"
#include "NetHandler.h"
#include "NStorDev_IF.h"
#include <string.h>

namespace Dahua{
namespace NetFramework{
	
class NETFRAMEWORK_API CNFile : public CNetHandler
{
//缺省的buffer大小为：128K*4
#define	DEFAULT_BUFFER_SIZE		(128*1024)
#define	DEFAULT_BUFFER_NUM		4
private:
	CNFile();
	~CNFile();
	CNFile( CNFile const & other );
	CNFile& operator=( CNFile const & other );
public:
	//创建一个CNFile对象，同时打开一个文件。
	//参数：recvr_id: 消息接收者的ID。
	//			path：文件路径，只要与CNStorDev_IF子类的Open的参数path意义一致即可
	//			mode：打开方式,与CNStorDev_IF子类的Open的参数mode意义一致即可
	//			dev： 文件对应存储设备，即实际具备文件存储能力的类
	//返回值：成功，则返回一个CNFile对象指针，失败则返回NULL。
	static CNFile* Open( long recvr_id, const char* path, const char* mode,CNStorDev_IF* dev = NULL );
	//关闭
	void Close();
	//写文件
	//参数：buf：数据缓冲区
	//			len：数据长度
	//返回值：大于等于0：实际写入量
	//若实际写入量小于需要写入的量len时，文件暂时无法再写入，
	//需要等待消息MSG_FILE_WRITE。
	//写的过程中，文件发生异常，则会收到MSG_FILE_EXCEPTION消息。
	int Write( const char* buf, const uint32_t len );
	//读文件
	//参数：buf：		缓冲区
	//			len：		缓冲区长度
	//			is_EOF: 是否已经读到文件尾，为true表示此次读取已经到达文件尾。
	//返回值：大于等于0：表示实际读取量
	//若返回值小于len，表示文件暂时无数据可读取，需要等待消息MSG_FILE_READ。
	//读的过程中，文件发生异常，则会收到MSG_FILE_EXCEPTION消息。
	int Read( char* buf, uint32_t len, bool& is_EOF );
	//定位文件读写指针
	//参数：offset：偏移量
	//			whence：定位起始点，允许值为FILE_HEAD, FILE_CUR.
	//返回值：0表示成功，-1表示失败。
	int Seek( int64_t offset, int whence );
	//获取文件状态
	//参数：state：保存状态的结构体
	//返回值：-1：失败，0：成功
	int Stat( struct NFile_Info* state );
	//设置文件缓冲区的大小，缺省为4个128K的缓冲区，共512K
	//参数： size_per_buffer, 每一个缓冲区的大小，该参数已无效，系统内部会自动优化，所以，随意传入一个值即可。
	//			 buffer_num, 缓冲区的数量，缺省4个。一般建议读文件设置为2个，写文件设置为8个或以上。
	//返回值：0成功，-1失败。
	int SetBufferSize( uint32_t size_per_buffer = DEFAULT_BUFFER_SIZE, 
										uint32_t buffer_num = DEFAULT_BUFFER_NUM );
private:
	struct Internal;
	struct Internal*	m_internal;
};

}//NetFramework
}//Dahua

#endif //__INCLUDED_DAHUA_NETFRAMEWORK_NFILE_H__
