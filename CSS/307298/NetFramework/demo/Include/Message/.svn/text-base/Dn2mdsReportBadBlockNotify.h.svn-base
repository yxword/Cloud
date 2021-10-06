//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_DN2MDS_REPORT_BAD_BLOCK_NOTIFY_H__
#define __INCLUDE_DAHUA_EFS_DN2MDS_REPORT_BAD_BLOCK_NOTIFY_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"
#include <vector>

namespace Dahua {
namespace EFS {
//类名 	   CDn2mdsReportBadBlockNotify
//协议类型 DS_PROTOCOL_NOTIFY
//协议名   DN2MDS_REPORT_BAD_BLOCK_NOTIFY
//发起方   DATANODE
//接收方   MDS
//描述	   DATANODE向MDS报告错误块信息


class CDn2mdsReportBadBlockNotify : public CPacketBase
{
public:
//坏块的类型
typedef enum BadBlockType {
    missingMetaFile = 0, //meta文件丢失,汇报不关心blocklength,datanode会删除残留信息
    missingBlockFile,	 //block文件丢失,汇报不关心blocklength，datanode会删除残留信息
    repairBlockLength,   //meta和block长度不一致,mds需要已新的blocklength为准
    verifyError			 //校验出错,datanode会删除残留信息
}BadBlockType;	

public:
	CDn2mdsReportBadBlockNotify( );
	CDn2mdsReportBadBlockNotify( uint32_t seq );
	~CDn2mdsReportBadBlockNotify( );

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

typedef Memory::TSmartObjectPtr<CDn2mdsReportBadBlockNotify> TDn2mdsReportBadBlockPtr;

TPacketBasePtr newCDn2mdsReportBadBlockNotify(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_DN2MDS_REPORT_BAD_BLOCK_NOTIFY_H__
