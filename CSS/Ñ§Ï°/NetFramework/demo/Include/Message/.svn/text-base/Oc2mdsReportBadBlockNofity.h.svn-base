//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_OC2MDS_REPORT_BAD_BLOCK_NOTIFY_H__
#define __INCLUDE_DAHUA_EFS_OC2MDS_REPORT_BAD_BLOCK_NOTIFY_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"
#include <vector>

namespace Dahua {
namespace EFS {
//类名 	   CReportBadBlockNofity
//协议类型 DS_PROTOCOL_NOTIFY
//协议名   OC2MDS_REPORT_BAD_BLOCK_NOTIFY
//发起方   客户端
//接收方   MDS
//描述	   客户端向MDS汇报发现的错误块

class COc2mdsReportBadBlockNofity : public CPacketBase
{
public:
//坏块的类型
typedef enum BadBlockType {
    missingBlock = 0,		//块不存在，汇报的块不需要关心blockLength
    blockLengthInconsistent	//块长度不一致，汇报的块需要填充真实的blockLength
}BadBlockType;

public:
	COc2mdsReportBadBlockNofity( );
	COc2mdsReportBadBlockNofity( uint32_t seq );
	~COc2mdsReportBadBlockNofity( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//协议内容
	//设置和获取错误类型
	void setBadType( int32_t type );
	int32_t getBadType( ) const;
	//设置和获取错误块列表
	void setBadBlock( std::vector<struct Block>& blocks );
	void getBadBlock( std::vector<struct Block>& blocks ) const;

private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<COc2mdsReportBadBlockNofity> TOc2mdsReportBadBlockPtr;

TPacketBasePtr newCOc2mdsReportBadBlockNofity(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_OC2MDS_REPORT_BAD_BLOCK_NOTIFY_H__
