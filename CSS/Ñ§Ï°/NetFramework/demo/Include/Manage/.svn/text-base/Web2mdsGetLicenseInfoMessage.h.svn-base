//
//
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_WEB2MDS_GET_LICENSE_INFO_MESSAGE_H__
#define __INCLUDE_DAHUA_WEB2MDS_GET_LICENSE_INFO_MESSAGE_H__

#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/PacketBase.h"

namespace Dahua {
namespace EFS {
//类名 	   CMlicenseInfo
//协议类型 DS_PROTOCOL_REQ
//协议名   GET_LICENSE_INFO
//发起方   客户端
//接收方   MDS
//描述	   客户端向MDS获取EFS系统信息

//类名 	   CResMlicenseInfo
//协议类型 DS_PROTOCOL_RES
//协议名   GET_LICENSE_INFO
//发起方   MDS
//接收方   客户端
//描述	   正常,MDS向客户端EFS系统信息

class CWeb2mdsGetLicenseInfoMessage: public CPacketBase {
public:
	CWeb2mdsGetLicenseInfoMessage();
	CWeb2mdsGetLicenseInfoMessage( uint32_t seq );
	~CWeb2mdsGetLicenseInfoMessage();

	//尾部插入方式增加原始数据( 序列化后的 )
	bool append( const char* buf, uint32_t len );
	//获取序列化的数据
	const char* getData( );
	//获取序列化的数据的长度
	uint32_t getDataLen( );
	//打印包用于debug
	void printPacket( ) const;

private:
	class Internel;
	Internel* m_internel;
};

class CResWeb2mdsGetLicenseInfoMessage: public CPacketBase{
public:
	CResWeb2mdsGetLicenseInfoMessage();
	CResWeb2mdsGetLicenseInfoMessage( uint32_t seq );
	~CResWeb2mdsGetLicenseInfoMessage();

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

	//获取license文件是否有错,1表示合法,0标识非法
    void setLicenseValid(int8_t status);
    int8_t getLicenseValid( ) const;
    
    //获取license文件设置的容量大小
    void setFeatureCapacity(int64_t capacity );
    int64_t getFeatureCapacity( ) const;
    
    //获取使用的容量是否超过license设置的容量，1表示没有超过,0表示超过容量限制
    void setFeatureCapStatus(int8_t capacityStatus );
    int8_t getFeatureCapStatus( ) const;    

private:
	class Internel;
	Internel *m_internel;
};

typedef Memory::TSmartObjectPtr<CWeb2mdsGetLicenseInfoMessage> TWeb2mdsGetLicenseInfoMsgPtr;
typedef Memory::TSmartObjectPtr<CResWeb2mdsGetLicenseInfoMessage> TResWeb2mdsGetLicenseInfoMsgPtr;

TPacketBasePtr newCWeb2mdsGetLicenseInfoMessage( );
TPacketBasePtr newCResWeb2mdsGetLicenseInfoMessage( );


} /* namespace EFS */
} /* namespace Dahua */
#endif /* __INCLUDE_DAHUA_WEB2MDS_GET_LICENSE_INFO_MESSAGE_H__ */
