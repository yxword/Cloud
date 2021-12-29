//
//  "$Id:"
//
//  Copyright (c)1992-2015, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2015-8-15		 he_wei  	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SSL_DGRAM_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SSL_DGRAM_H__

#include "NDefs.h"
#include "SockAddr.h"
#include "SockDgram.h"

namespace Dahua {
namespace NetFramework{

class NETFRAMEWORK_API CSslDgram : public CSock
{
public:
	CSslDgram();
	~CSslDgram();
public:
	// 该接口用于ticket-session操作的初始化，用于生成一组对ticket进行加解密的密钥。
	// 返回值: 0
	static int Init();
public:
///非阻塞
	/// 以下两个接口用于执行非阻塞的Accept的操作。
	//打开服务端的本地端口
	//参数: addr 本地端口地址， reuseaddr 该端口是否可重用
	//返回值: -1 操作失败  0 操作成功
	int Open( const CSockAddr *addr, bool reuseaddr = false );
	//用于非阻塞的执行接口连接请求操作
	//参数: remote 是否指定请求地址
	//返回值：-1 操作失败 0 操作成功
	int AcceptX( const CSockAddr *remote = NULL );
///阻塞
	// 阻塞的接口远端的链接请求
	// 参数: addr 本地监听端口， reuseaddr 该端口是否可重用 remote　指定的请求地址
	// 返回值: -1 操作失败 0 操作成功
	int Accept( const CSockAddr *addr, bool reuseaddr = false, const CSockAddr *remote = NULL );
public:
	// 请求建立SSL/TLS链接 ( 阻塞操作 )
	// 参数: remote　远程服务端地址，local 本地地址
	// 返回值: -1 链接失败， 0 链接成功
	int Connect( const CSockAddr *remote, const CSockAddr* local = NULL );
public:
	// 设置SSL/TLS链接选项
	// 参数: option 配置项 type 配置项类型 para 配置项参数
	// 返回值: 0
	int SetOption(int option, int type, void* para);
public:
	//发送UDP数据报
	//参数：buf 待发送的数据，len 待发送数据的长度
	//返回值：-1 失败 0 未发送出去，下次重试 大于0的值表示已发送的长度。
	int Send( const char * buf, uint32_t len );
	//接收UDP数据报
	//参数：buf 接收数据的缓冲 len 接收数据缓冲的大小
	//返回值：-1 失败 0 未接收到，下次重试 大于0的值表示已接收到的长度。
	int Recv(char * buf, uint32_t len );
private:
	struct Internal*	m_internal;
};

}; // NetFramework
}; // Dahua

#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SSL_DGRAM_H__
