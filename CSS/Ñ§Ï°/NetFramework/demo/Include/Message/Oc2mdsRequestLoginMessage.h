//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_OC2MDS_REQUEST_LOGIN_MESSAGE_H__
#define __INCLUDE_DAHUA_EFS_OC2MDS_REQUEST_LOGIN_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/PacketBase.h"
#include "Common/StructBase.h"

namespace Dahua {
namespace EFS {

//类名 	   COc2mdsRequestLoginMessage
//协议类型 DS_PROTOCOL_REQ
//协议名   OC2MDS_REQUEST_LOGIN_MESSAGE
//发起方   客户端
//接收方   MDS
//描述	   客户端向MDS申请登陆

//类名 	   CResOc2mdsRequestLoginMessage
//协议类型 DS_PROTOCOL_RES
//协议名   OC2MDS_REQUEST_LOGIN_MESSAGE
//发起方   MDS
//接收方   客户端
//描述	   正常,MDS向客户端响应登陆信息;出错,响应错误信息

class COc2mdsRequestLoginMessage : public CPacketBase
{
public:
	COc2mdsRequestLoginMessage( );
	COc2mdsRequestLoginMessage( uint32_t seq );
	~COc2mdsRequestLoginMessage( );

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

	//协议内容
	//设置客户端名字
	void setClientName(const std::string& clientName);
	//获取客户端名字
	std::string getClientName() const;
	//设置用户
	//第一次不用设置用户,而是在取得加密的key后，通过加密再次请求登陆时设置
	void setUser(const std::string& userName, const std::string& password);
	//获取用户
	void getUser(std::string& userName, std::string& password) const; 

private:
	class Internel;
	Internel *m_internel;
};


class CResOc2mdsRequestLoginMessage : public CPacketBase
{
public:
	CResOc2mdsRequestLoginMessage( );
	CResOc2mdsRequestLoginMessage( uint32_t seq );
	~CResOc2mdsRequestLoginMessage( );

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
	
	//在客户端第二次登陆后,响应客户端不需要填充下面信息
	//设置和获取加密类型
	void setEncryType(const std::string& encryType);
	std::string getEncryType() const;
	//设置和获取域
	void setRealm(const std::string& realm);
	std::string getRealm() const;
	//设置和获取随机密钥
	void setRandomKey(const std::string& rand);
	std::string getRandomKey() const;

	void setSupportECType(const std::vector<struct ErasureMapping>& mapping);
	void getSupportECType(std::vector<struct ErasureMapping>& mapping) const;

private:
	class Internel; 
	Internel *m_internel;
};	

typedef Memory::TSmartObjectPtr<COc2mdsRequestLoginMessage> TOc2mdsRequestLoginMsgPtr;
typedef Memory::TSmartObjectPtr<CResOc2mdsRequestLoginMessage> TResOc2mdsRequestLoginMsgPtr;

TPacketBasePtr newCOc2mdsRequestLoginMessage(  );
TPacketBasePtr newCResOc2mdsRequestLoginMessage(  );

} // namespace EFS
} // namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_OC2MDS_REQUEST_LOGIN_MESSAGE_H__
