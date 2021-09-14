//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_CLOUD_STORAGE_DN2MDS_REPORT_BLOCK_MESSAGE_H__
#define __INCLUDE_DAHUA_CLOUD_STORAGE_DN2MDS_REPORT_BLOCK_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"
#include <vector>

namespace Dahua {
namespace EFS {
//类名 	   CDn2mdsReportBlockMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   DN2MDS_REPORT_BLOCK_MESSAGE
//发起方   DATANODE
//接收方   MDS
//描述	   DATANODE向MDS发送块汇报信息

class CDn2mdsReportBlockMessage : public CPacketBase
{
public:
	enum Pattern {
		passive = 0, //默认为被动
		active  = 1  //主动
	};
	enum ReportStatus {
		partAdd = 0, //默认为部分增加
		addAll = 1,  //全量增加
		reduce = 2,  //减少
	};
public:
	CDn2mdsReportBlockMessage( );
	CDn2mdsReportBlockMessage( uint32_t seq );
	~CDn2mdsReportBlockMessage( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//协议内容
	//设置和获取主动或被动模式
	void setPattern(uint8_t mode);
	uint8_t getPattern() const;
	
	//设置和获取汇报的状态
	void setStatus( uint8_t status );
	uint8_t getStatus() const;

	//设置和获取需要汇报的总数
	void setTotalReportNum( int32_t total );
	int32_t getTotalReportNum( ) const;
	//设置和获取当前汇报中最大的同步索引
	void setSyncIndex( uint32_t syncIndex );
	uint32_t getSyncIndex( ) const;
	//设置和获取当前这批在总汇报中的开始索引位置
	void setStartIndex( int32_t cix );
	int32_t getStartIndex( ) const;
	//设置和获取当前汇报的个数
	void setCurrentReportNum(int32_t num);
	int32_t getCurrentReportNum() const;
	
	//设置和获取需要汇报的blocks
	void setReportBlocks( std::vector<struct Block>& blocks );
	void getReportBlocks( std::vector<struct Block>& blocks ) const;
private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CDn2mdsReportBlockMessage> TDn2mdsReportBlockMsgPtr;

TPacketBasePtr newCDn2mdsReportBlockMessage(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_CLOUD_STORAGE_DN2MDS_REPORT_BLOCK_MESSAGE_H__
