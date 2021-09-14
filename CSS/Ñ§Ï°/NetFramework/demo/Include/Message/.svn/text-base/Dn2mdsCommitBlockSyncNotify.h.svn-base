//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_DN2MDS_COMMIT_BLOCK_SYNC_NOTIFY_H__
#define __INCLUDE_DAHUA_EFS_DN2MDS_COMMIT_BLOCK_SYNC_NOTIFY_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"
#include <vector>

namespace Dahua {
namespace EFS {
//类名 	   CDn2mdsCommitBlockSyncNotify
//协议类型 DS_PROTOCOL_NOTIFY
//协议名   DN2MDS_COMMIT_BLOCK_SYNC_NOTIFY
//发起方   DATANODE
//接收方   MDS
//描述	   DATANODE向MDS汇报块恢复结果信息

class CDn2mdsCommitBlockSyncNotify : public CPacketBase
{
 public:
    // 结构体放到类内部，以限制范围
    struct RecoverResult{
        bool result; // true 恢复成功，fail 恢复失败
        struct Block block; // 恢复结果的block信息
        struct DataNodeId blockPlacesLocation; // 恢复结果所在的datanode信息
    };
public:
	CDn2mdsCommitBlockSyncNotify( );
	CDn2mdsCommitBlockSyncNotify( uint32_t seq );
	~CDn2mdsCommitBlockSyncNotify( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//设置和获取块恢复结果
	//恢复失败get返回false,取出的Block中的blockIndex和objOffset为准确的信息
	//恢复成功get返回true,取出去的Block信息都为准确信息
	void setRecoverResult(const std::vector<struct RecoverResult>& res);
	void getRecoverResult(std::vector<struct RecoverResult>& res) const;

private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CDn2mdsCommitBlockSyncNotify> TDn2mdsCommitBlockSyncPtr;
TPacketBasePtr newCDn2mdsCommitBlockSyncNotify(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_DN2MDS_COMMIT_BLOCK_SYNC_NOTIFY_H__
