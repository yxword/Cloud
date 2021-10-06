//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_DN2MDS_DISK_BROKEN_NOTIFY_H__
#define __INCLUDE_DAHUA_EFS_DN2MDS_DISK_BROKEN_NOTIFY_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"
#include <vector>

namespace Dahua {
namespace EFS {
//类名 	   CDn2mdsDiskBrokenNotify
//协议类型 DS_PROTOCOL_NOTIFY
//协议名   DN2MDS_DISK_BROKEN_NOTIFY
//发起方   DATANODE
//接收方   MDS
//描述	   DATANODE向MDS汇报异常磁盘信息

class CDn2mdsDiskBrokenNotify : public CPacketBase
{
public:
	CDn2mdsDiskBrokenNotify( );
	CDn2mdsDiskBrokenNotify( uint32_t seq );
	~CDn2mdsDiskBrokenNotify( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//设置获取异常磁盘数
	void setBrokens(uint32_t disks);
	uint32_t getBrokens() const;

	//设置和获取磁盘总数
	void setDiskTotalNum(uint32_t total);
	uint32_t getDiskTotalNum() const;

private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CDn2mdsDiskBrokenNotify> TDn2mdsDiskBrokenPtr;
TPacketBasePtr newCDn2mdsDiskBrokenNotify(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_DN2MDS_DISK_BROKEN_NOTIFY_H__
