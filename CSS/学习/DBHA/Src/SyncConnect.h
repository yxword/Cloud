//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:    主备同步模块服务端收到的链接.由CSyncServer管理.
//	Revisions:		2017-1-5  mao_jijia
//


#ifndef __DAHUA_VIDEOCLOUD_DBHA_SYNCCONNECT_H__
#define __DAHUA_VIDEOCLOUD_DBHA_SYNCCONNECT_H__

#include "Common/ServiceBase.h"
#include "Memory/SharedPtr.h"
#include "Infra/Function.h"

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

class CSyncConnect : public EFS::CServiceBase
{
public:
	typedef Infra::TFunction0<void> CloseProc;
    typedef Infra::TFunction2<void, EFS::TPacketBasePtr, EFS::TPacketBasePtr&> PacketProc;
	CSyncConnect( CloseProc closeProc, PacketProc pktProc );
	~CSyncConnect( void );

	virtual void handleTimeout( int32_t id );
	virtual void handleException();
	virtual void handleClose();

protected:
    void onPacket( EFS::TPacketBasePtr req, EFS::TPacketBasePtr& rsp );

private:
	CloseProc m_close_proc;
	PacketProc m_packet_proc;
	uint32_t m_is_unrecved_packet; //多少个心跳周期没有收到包.
};

} // DBHA 
} // VideoCloud
} // Dahua

#endif // __DAHUA_VIDEOCLOUD_DBHA_SYNCCONNECT_H__
