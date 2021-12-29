#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SSL_OPTION_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SSL_OPTION_H__

#include "SslX509.h"
namespace Dahua {
namespace NetFramework{

/// 配置项说明
enum{
		Module_Session = 0,
		Module_X509,
		Protocol_Ban,
		CipherList_Rule,
		Module_Timeout,
		Privkey_Key,
		OptionTypeTotal
};

///  配置项结构
struct Option{
	int		type;
	union {
		int value;   // for Session 0: ip+port 1: only ip, and also only for dtls1.
		Dahua::NetFramework::CSslX509* x509;
		//NULL;
		char* command;
	} data;
};

/// 配置项种类说明
//#define  Module_Session
	#define  Type_Session_none  0
	#define  Type_Session_id    1
	#define	 Type_Session_ticket  2
	// para   但这两种类型尚未实现,还是以IP+PORT形式为主
	#define  Session_Class_Type_IP_PORT 0
	#define  Session_Class_Type_ONLY_IP 1
	// Type_Session_id_control = Type_Session_id | 0b100
	// para 等于NULL时,启用外部session控制,即每次连接生成SSL_SESSION,传输相关ID给外部保存。
	// 外部可通过ID设设置,para传输ID给内部,可进行session的复用,否则不进行相关复用。
	#define  Type_Session_control	4   // do not use this.
	#define  Type_Session_id_control	Type_Session_control | Type_Session_id
	#define  Type_Session_ticket_control	Type_Session_control | Type_Session_ticket

//#define  Module_X509
	#define Type_X509_none 0
	#define Type_X509_verify 1
	#define Type_X509_input 2
	#define Type_X509_all Type_X509_verify | Type_X509_input

//#define	Protocol_Ban
	#define Type_Protocol_Ban_none	 0
	#define Type_Protocol_Ban_SSLv2  1
	#define Type_Protocol_Ban_SSLv3  2
	#define Type_Protocol_Ban_TLSv1_0  4
	#define Type_Protocol_Ban_TLSv1_1  8

//#define	CipherList_Rule
	#define Type_CipherList_Rule_none 0
	#define Type_CipherList_Rule_set 1

//#define Module_Timeout
	#define Type_Connect_timeout   0
	#define Type_SSL_HandShake_timeout   1		// 设置HTTPS握手过程中read的超时时间，参数单位是ms

//#define	Privkey_Key
	#define Type_Privkey_Key	0

}	// NetFramework
}	// Dahua

#endif // #ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SSL_OPTION_H__
