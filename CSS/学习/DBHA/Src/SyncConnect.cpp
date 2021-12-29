#include "SyncConnect.h"
#include "MessageType.h"
#include "HAHeartbeatMessage.h"
#include "SyncBeginMessage.h"
#include "SyncDataMessage.h"
#include "SyncEndMessage.h"
#include "SyncDumpFileInfoMessage.h"
#include "Common/ProtoParser.h"
#include "HAConfigs.h"
#include "HALog.h"

namespace Dahua {
namespace VideoCloud {
namespace DBHA {

using namespace  EFS;

DBHALOG_CLASSNAME( CSyncConnect );

CSyncConnect::CSyncConnect( CloseProc closeProc, PacketProc pktProc )
{
	m_close_proc = closeProc;
	m_packet_proc = pktProc;

	//注册相关协议.
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_REQ, HAFK_SYNC_BEGIN_MESSAGE, newCSyncBeginMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_RES, HAFK_SYNC_BEGIN_MESSAGE, newCResSyncBeginMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_REQ, HAFK_SYNC_DATA_MESSAGE, newCSyncDataMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_RES, HAFK_SYNC_DATA_MESSAGE, newCResSyncDataMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_REQ, HAFK_SYNC_END_MESSAGE, newCSyncEndMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_RES, HAFK_SYNC_END_MESSAGE, newCResSyncEndMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_REQ, HAFK_SYNC_DUMP_FILE_INFO_MESSAGE, newCSyncDumpFileInfoMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_RES, HAFK_SYNC_DUMP_FILE_INFO_MESSAGE, newCResSyncDumpFileInfoMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_REQ, HAFK_HEARTBEAT_MESSAGE, newCHAHeartbeatMessage );
	EFS::CProtoParser::setPacketFactory( EFS::DS_PROTOCOL_RES, HAFK_HEARTBEAT_MESSAGE, newCResHAHeartbeatMessage );

	addMethod( EFS::DS_PROTOCOL_RES, HAFK_SYNC_BEGIN_MESSAGE, ( CServiceBase::Method )&CSyncConnect::onPacket );
	addMethod( EFS::DS_PROTOCOL_RES, HAFK_SYNC_DATA_MESSAGE, ( CServiceBase::Method )&CSyncConnect::onPacket );
	addMethod( EFS::DS_PROTOCOL_RES, HAFK_SYNC_END_MESSAGE, ( CServiceBase::Method )&CSyncConnect::onPacket );
	addMethod( EFS::DS_PROTOCOL_RES, HAFK_SYNC_DUMP_FILE_INFO_MESSAGE, ( CServiceBase::Method )&CSyncConnect::onPacket );
	addMethod( EFS::DS_PROTOCOL_REQ, HAFK_HEARTBEAT_MESSAGE, ( CServiceBase::Method )&CSyncConnect::onPacket );
	addMethod( EFS::DS_PROTOCOL_REQ, HAFK_SYNC_NOTIFY_RECOVER_FINISH, ( CServiceBase::Method )&CSyncConnect::onPacket );

	m_is_unrecved_packet = 0;
	setTimer( gHAConfig.getSyncHeartbeatInterval() );
}

CSyncConnect::~CSyncConnect( void )
{

}

void CSyncConnect::handleException()
{
	DBHALOG_INFO( "sync connect handle exception." );
	close();
}

void CSyncConnect::handleClose()
{
	DBHALOG_INFO( "sync connect handle close." );
	m_close_proc();
}

void CSyncConnect::onPacket( EFS::TPacketBasePtr req, EFS::TPacketBasePtr& rsp )
{
	m_is_unrecved_packet = 0;
	m_packet_proc( req, rsp );
}

void CSyncConnect::handleTimeout( int32_t id )
{
	m_is_unrecved_packet++;
	if( m_is_unrecved_packet>gHAConfig.getSyncHeartbeatTimeout() ){
		DBHALOG_ERROR( "handle timeout." );
		destroyTimer( id );
		close();
	}
}

} // DBHA 
} // VideoCloud
} // Dahua
