//
//  "$Id: SslAcceptor.h 7672 2010-5-27 02:28:18Z zmw $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-27		zmw        	Create
//					2015-12-4       hw          Rebuild

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SSL_ACCEPTOR_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SSL_ACCEPTOR_H__

#include "NDefs.h"
#include "SslStream.h"
#include "SockAddr.h"
#include <string.h>

namespace Dahua{
namespace NetFramework{

class NETFRAMEWORK_API CSslAcceptor : public CSock
{
	CSslAcceptor& operator=(CSslAcceptor const& other);
	CSslAcceptor(CSslAcceptor const& other);
public:
	CSslAcceptor();
	~CSslAcceptor();
public:
/// 老接口,前向兼容。
	//设置ssl密钥文件cacert.pem和privkey.pem的路径，需要设置为系统路径
	//参数：path 系统密钥文件cacert.pem和privkey.pem的绝对路径
	//返回值：-1 失败 0 成功
	int SetPemPath( const char* path );
	//设置是否验证对端的数字证书, 在Accept()之前调用
	//该接口的优先级高于SetOption,请尽量避免使用该接口
	//参数: flag  是否验证对端数字证书
	void SetCertVerityPeer( bool flag );
public:
	//服务器端侦听本地端口
	//参数：local 本地地址信息，ip和端口
	//返回值：-1失败  0 成功
	int Open( const CSockAddr & local );
	//接受一个远端的连接请求，主动生成CSslStream,外部需要主动侧该CSslStream.
	//参数：remote 调用后记录连接请求的客户端地址信息
	//返回值：CSslStream类型指针，NULL 失败，否则成功
	CSslStream* Accept( CSockAddr * remote = NULL );
	//接受一个远端的连接请求, 需要外部输入CSslStream.
	//参数：sock用于填充接受的连接请求
	//		remote远端地址
	//返回值：-1：接收失败，sock不可用 0：成功，sock可用
	int Accept( CSslStream& sock, CSockAddr * remote = NULL );

	//接受一个远端的SSL连接请求,不用调用Open()
	//参数：sock 用于填充接受的连接请求
	//		stream 已经建立TCP连接的CSockStream指针
	//返回值：-1：接收失败，sock不可用 0：成功，sock可用
	int Accept(CSslStream& sock, CSockStream* stream);
public:
	//设置SSL链接配置选项,需在Accept操作前使用。
	//保留性老接口,不增加新功能,仅供compatibility。
	//参数: option  配置名  type 该配置的类型 para 该配置相关的参数
	//返回值: 0
	int SetOption(int option, int type, void* para);
	/// 增加 para的长度值,解决二进制参数长度不确定问题
	// 设置SSL链接的属性,在Connect操作之前使用。
	///\param[in] option  属性项名称
	///\param[in] type    属性项类别
	///\param[in] para    属性项参数
	///\param[in] len     属性参数内存块长度
	int SetOption(int option, int type, void* para, int len);
private:
	struct Internal*	m_internal;
};

}//namespace NetFramework
}//namespace Dahua

#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SSL_ACCEPTOR_H__
