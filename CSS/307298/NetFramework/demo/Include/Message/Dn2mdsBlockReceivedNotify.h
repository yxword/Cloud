//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_DN2MDS_BLOCK_RECEIVED_NOTIFY_H__
#define __INCLUDE_DAHUA_EFS_DN2MDS_BLOCK_RECEIVED_NOTIFY_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"
#include <vector>

namespace Dahua {
namespace EFS {
//类名 	   CDn2mdsBlockReceivedNotify
//协议类型 DS_PROTOCOL_NOTIFY
//协议名   DN2MDS_BLOCK_RECEIVED_NOTIFY
//发起方   DATANODE
//接收方   MDS
//描述	   DATANODE向MDS汇报接收到的文件


class CDn2mdsBlockReceivedNotify : public CPacketBase
{
public:
	CDn2mdsBlockReceivedNotify( );
	CDn2mdsBlockReceivedNotify( uint32_t seq );
	~CDn2mdsBlockReceivedNotify( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//协议内容
	//设置和获取注册信息
	//void setDataNodeRegistration( struct DatanodeRegistration& reg );
	//void getDataNodeRegistration( struct DatanodeRegistration& reg ) const;

	//设置和获取最大的maxSyncId
	void setSyncId(uint32_t syncId);
	uint32_t getSyncId() const;

	//设置和获取汇报的block
	void setReceivedBlock( std::vector<struct Block>& blocks );
	void getReceivedBlock( std::vector<struct Block>& blocks ) const;

private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CDn2mdsBlockReceivedNotify> TDn2mdsBlockReceivedPtr;

TPacketBasePtr newCDn2mdsBlockReceivedNotify(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_DN2MDS_BLOCK_RECEIVED_NOTIFY_H__
