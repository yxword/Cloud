//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_CLOUD_EFS_MESSAGEBASE_H__
#define __INCLUDE_DAHUA_CLOUD_EFS_MESSAGEBASE_H__

#include "Infra/IntTypes.h"
#include "Memory/SmartObjectPtr.h"
#include "Infra/AtomicCount.h"
#include "Serialization.h"
#include "Protocol.h"
#ifndef WIN32
#include <unistd.h>
#endif
#include <assert.h>
#include <string.h>

namespace Dahua {
namespace EFS {

#define PROTO_VERSION    PROTO_VERSION_V1
	
typedef struct ProtoHeader
{
	ProtoHeader()
	{
		version = PROTO_VERSION;
		reserved = 0;
		magic[0] = 'D';
		magic[1] = 'H';
		magic[2] = 'C';
		magic[3] = 'S';

		type = 0;
		cmd = 0;
		seq = 0;
		bodyLen = 0;
	}
	uint8_t		version;
	uint8_t		type;
	int16_t		cmd;
	uint32_t	seq;
	uint32_t	bodyLen:22;
	uint32_t	reserved:10;
	uint8_t		magic[4];
}ProtoHeader;

//协议消息基类
class CPacketBase
{	
public:
	CPacketBase();
	virtual ~CPacketBase();
	//释放Packet，需要和创建方式匹配
	virtual void destroy(){ delete this; }
	//输入字节流，反序列化成消息体(支持追加方式)
	virtual bool append( const char* buf, uint32_t len ) = 0; 
	//获得消息的序列化后字节流
	virtual const char* getData() = 0;
	//获得消息序列化字节流长度
	virtual uint32_t getDataLen() = 0;

	//打印packet内容，调试使用
	virtual void printPacket() const = 0;  
	//设置和获取私有数据，可以作为承载上下文执行环境之用
	void setPriData( void* data );
	void getPriData( void** data );
public:
	inline void setSequence( uint32_t seq ){ m_header.seq = seq; }
	inline uint8_t getVersion()const { return m_header.version; }
	inline uint8_t getPktType()const { return m_header.type; }
	inline uint16_t getPktCmd()const { return m_header.cmd; }
	inline uint32_t getSequence()const { return m_header.seq; }
	inline uint32_t getBodyLength()const { return m_header.bodyLen; }
protected:
	bool set_proto_header( const char* header, uint32_t len );	
	ProtoHeader	m_header;
private:
	struct Internal;
	struct Internal* m_internal;
};

#define PROTO_HEAD_SIZE  sizeof(ProtoHeader)

typedef Memory::TSmartObjectPtr<CPacketBase> TPacketBasePtr;

//各种Pakcet工厂原型
typedef TPacketBasePtr (*pkt_builder)();

//声明，供TSmartObjectPtr调用
void addObjectRef(CPacketBase* p);
void releaseObject(CPacketBase* p);

} // namespace CloudStorage
} // namespace Dahua

#endif //__INCLUDE_DAHUA_CLOUD_EFS_MESSAGEBASE_H__
