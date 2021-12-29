//
//  "$Id: SslStream.h 7672 2010-5-27 02:28:18Z zmw $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-27		zmw         	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SSL_STREAM_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SSL_STREAM_H__

#include "NDefs.h"
#include "Infra/IntTypes.h"
#include "SockAddr.h"
#include "SockStream.h"
#include "SslX509.h"
#ifndef WIN32
#include <sys/uio.h>
#endif

namespace Dahua{
namespace NetFramework{

class NETFRAMEWORK_API CSslStream : public CSock
{
	CSslStream& operator=(CSslStream const& other);
	CSslStream(CSslStream const& other);
public:
	CSslStream();
	~CSslStream();
public:
/// 老接口，前向兼容。
	//设置ssl密钥文件cacert.pem和privkey.pem的路径，需要设置为系统路径
	//参数：path 系统密钥文件cacert.pem和privkey.pem的绝对路径
	//返回值：-1 失败 0 成功
	int SetPemPath( const char* path );
	// 获取SSL交互中，对端携带的数字证书信息
	//返回值： <0 失败 0 成功
	int GetPeerCert( CSslX509* x509 );
public:
	//请求ssl链接远端服务器
	//参数：remote 远端服务端的地址，包括IP和端口
	//返回值：-1 失败  0 成功
	int Connect(const CSockAddr & remote,CSockAddr* local = NULL);
	//在已经TCP握手情况下进行ssl握手
	//参数：sock 已经建立TCP连接的CSockStream指针
	//返回值：-1失败 0 成功
	int Connect( CSockStream* sock );
	//发送数据
	//参数：buf 待发送缓冲区 len 待发送数据的字节数
	//返回值：-1 失败 0 未发送出去 大于0的值表示已发送的长度
	int Send( const char * buf, uint32_t len );
	//接收数据
	//参数：buf 接收数据的缓冲 len 接收数据缓冲的大小
	//返回值：-1 失败 0 未接收数据 大于0 已接收数据字节数
	int Recv( char * buf, uint32_t len );
	//发送TCP数据，
	//参数；同Linux系统调用writev的参数。
	//返回值：-1失败，0未发送下次再试，大于0表示发送出去的长度。
	int WriteV( const struct iovec *vector, int count );
	//带有超时的阻塞发送，接收，同样需要设置套接字为非阻塞模式。
	//超时时间单位为微秒，缺省为1秒。
	//参数：buf 待发送缓冲区 len 待发送数据的字节数 timeout 超时时间，默认为1秒
	//返回值：-1 失败 0  未发送数据 大于0 超时时间内发送的数据量
	int Send_n( const char * buf, uint32_t len, int timeout = 1000000 );
	//带超时的接收
	//参数：buf 接收缓冲区 len 接收缓冲区长度 timeout 超时时间，默认1秒
	//返回值：-1 失败 0 未接收到数据 大于0 实际接收到的数据量
	int Recv_n( char * buf, uint32_t len, int timeout = 1000000 );
	//从TCP栈中获取数据至SSL缓存区，并返回当前缓冲长度。如果未接收完SSL缓冲中peek值长度的数据，不会再从TCP栈中接收新数据。
	//该接口仅和Recv()接口结合使用, 在Recv()之前使用。
	//参数:  无
	//返回值：-1 失败, >=0 SSL缓冲区内数据的长度
	int Peek();
public:
	//设置SSL链接的属性,在Connect操作之前使用。
	//保留性老接口,不增加新功能,仅供compatibility。
	//参数：option  属性项名称
	//		type    属性项类别
	//      para    属性项参数
	int SetOption(int option, int type, void* para);
	/// 增加 para的长度值,解决二进制参数长度不确定问题
	// 设置SSL链接的属性,在Connect操作之前使用。
	///\param[in] option  属性项名称
	///\param[in] type    属性项类别
	///\param[in] para    属性项参数
	///\param[in] len     属性参数内存块长度
	int SetOption(int option, int type, void* para, int len);
	/// 获取SSL链接的属性,推荐在链接建立好之后使用
	///\param[in] option  属性项名称
	///\param[in/out] type    属性项类别
	///\param[out] para   属性项参数，内存由调用者分配
	///\param[in/out] len 分配内存块长度/实际内存块长度
	int GetOption(int option, int& type, void* para, int& len);
private:
	struct Internal*	m_internal;
};

}//NetFramework
}//Dahua

#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SSL_STREAM_H__

