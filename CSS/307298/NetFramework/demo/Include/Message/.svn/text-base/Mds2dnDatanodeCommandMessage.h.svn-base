//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_MDS2DN_DATANODE_COMMAND_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_MDS2DN_DATANODE_COMMAND_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"
#include "DatanodeCommand.h"
#include "Memory/SharedPtr.h"

namespace Dahua {
namespace EFS {

class CMds2dnDatanodeCommandMessage : public CPacketBase 
{
public:	
	CMds2dnDatanodeCommandMessage();
	CMds2dnDatanodeCommandMessage( uint32_t seq );
	~CMds2dnDatanodeCommandMessage();

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData();
	//获取序列化的数据的长度
	uint32_t getDataLen();
	//打印包用于debug
	void printPacket() const;

	//可多次调用add接口,增加command消息
	void add( const Memory::TSharedPtr<CDatanodeCommand>& cmds );
	void add( const std::vector<Memory::TSharedPtr<CDatanodeCommand> >& cmds );
	std::vector<Memory::TSharedPtr<CDatanodeCommand> > get();

protected:
	class Internel; 
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CMds2dnDatanodeCommandMessage> TMds2dnDatanodeCommandMsgPtr;
TPacketBasePtr newCMds2dnDatanodeCommandMessage( );

} //namespace EFS
} //namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_MDS2DN_DATANODE_COMMAND_MESSAGE_H__
