//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_OC2MDS_RENEW_LEASE_NOTIFY_H__
#define __INCLUDE_DAHUA_EFS_OC2MDS_RENEW_LEASE_NOTIFY_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {
//类名 	   COc2mdsRenewLeaseNotify
//协议类型 DS_PROTOCOL_NOTIFY
//协议名   OC2MDS_RENEW_LEASE_NOTIFY
//发起方   客户端
//接收方   MDS
//描述	   客户端向MDS续约

class COc2mdsRenewLeaseNotify : public CPacketBase
{
public:
	COc2mdsRenewLeaseNotify( );
	COc2mdsRenewLeaseNotify( uint32_t seq );
	~COc2mdsRenewLeaseNotify( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//协议内容
	//空
private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<COc2mdsRenewLeaseNotify> TOc2mdsRenewLeasePtr;

TPacketBasePtr newCOc2mdsRenewLeaseNotify(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_OC2MDS_RENEW_LEASE_NOTIFY_H__
