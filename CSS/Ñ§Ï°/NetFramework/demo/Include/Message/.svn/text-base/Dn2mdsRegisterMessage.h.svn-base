//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_DN2MDS_REGISTER_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_DN2MDS_REGISTER_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"

namespace Dahua {
namespace EFS {
//类名 	   CDn2mdsRegisterMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   DN2MDS_REGISTER_MESSAGE
//发起方   DATANODE
//接收方   MDS
//描述	   DATANODE向MDS注册


class CDn2mdsRegisterMessage : public CPacketBase
{
public:
	typedef enum{
		dahuaFamily = 0,     // 大华硬件节点
		otherFamily 	     // 第三方硬件节点
	}DNFamilyType;

	CDn2mdsRegisterMessage( );
	CDn2mdsRegisterMessage( uint32_t seq );
	~CDn2mdsRegisterMessage( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//协议内容
	//设置和获取token
	void setToken( const std::string& token );
	bool hasToken() const;
	std::string getToken() const;

	void setDataNodeRegistration( struct DatanodeRegistration& reg );
	void getDataNodeRegistration( struct DatanodeRegistration& reg ) const;

	void setDataNodeFamily( int32_t family );
	int32_t getDataNodeFamily( void ) const;
private:
	class Internel;
	Internel *m_internel;
};

class CResDn2mdsRegisterMessage : public CPacketBase
{
public:
	CResDn2mdsRegisterMessage( );
	CResDn2mdsRegisterMessage( uint32_t seq );
	~CResDn2mdsRegisterMessage( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;
	
	//协议内容
	//包有错误,则设置错误类型进去,否则默认包是正确的
	void setErrorType( int32_t errorType );
	//获取包是否有错,0表示正确,-1标识错误
	int8_t isError( ) const;
	//如果包出错,则获取到的错误类型标识是什么错误
	int32_t getErrorType( ) const;

	//设置cluster id
	void setClusterId( uint8_t id );
	uint8_t getClusterId() const;

	void setDataNodeRegistration( struct DatanodeRegistration& reg );
	void getDataNodeRegistration( struct DatanodeRegistration& reg ) const;
private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CDn2mdsRegisterMessage> TDn2mdsRegisterMsgPtr;
typedef Memory::TSmartObjectPtr<CResDn2mdsRegisterMessage> TResDn2mdsRegisterMsgPtr;

TPacketBasePtr newCDn2mdsRegisterMessage();
TPacketBasePtr newCResDn2mdsRegisterMessage();

}//EFS
}//Dahua

#endif //__INCLUDE_DAHUA_EFS_DN2MDS_REGISTER_MESSAGE_H__

