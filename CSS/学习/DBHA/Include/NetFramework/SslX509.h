
#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SSL_X509_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SSL_X509_H__

#include "NDefs.h"

namespace Dahua{
namespace NetFramework{

class CSslStream;

// 默认操作，不做任何处理。
#define CHECK_CHAIN_NONE 0
// 检查数字证书链时，同时删除不能构成证书链的多余证书。
#define CHECK_CHAIN_DELETE_SURPLUS 1

/// 数字证书信息, 512字节
struct CertSubjectMsg
{
	char 	country[4];				// 国家   填写 2个字节 NID_countryName
	char 	state_province[64];		// 州名或省名 NID_stateOrProvinceName
	char 	locatity[64];			// 位置 NID_localityName
	char 	organization[64];		// 组织名称 NID_organizationName
	char 	organization_unit[64];	// 组织单元名称 NID_organizationalUnitName
	char 	common_name[128];		// CN字段，与域名一致 NID_commonName
	char	reserved[124];			// 保留字节
};

class NETFRAMEWORK_API CSslX509
{
public:
	CSslX509();
	~CSslX509();
public:
	//获取对端数字证书的序列号
	//参数： buf 输出缓存，二进制值  len 输入时，缓存长度 输出时，实际长度
	//返回值： <0 失败 0 成功
	int GetCertSN( unsigned char* buf, int& len );
	//获取对端数字证书的使用者信息各个字段
	//参数： cert_sub_msg用于存储证书使用者信息的构体指针
	//返回值： <0 失败 0 成功
	int GetCertSubjectMsg(struct CertSubjectMsg *cert_sub_msg);
public:
	// 设置用户证书
	// 参数: cer 证书文件路径, prikey 密钥文件路径
	// 返回值: 0 数据读取成功，-1 证书文件异常， -2 私钥文件异常
	int SetCert( const char* cer, const char* privkey);
	// 设置本地用户证书的上级CA证书
	// 参数: ca 证书文件路径,
	// 返回值: 0 数据读取成功，-1 证书文件异常
	int SetCA( const char* ca );
	//验证输入的本地用户证书和CA证书是否组成数字证书链
	//参数: flag CHECK_CHAIN_NONE 不做任何处理，CHECK_CHAIN_DELETE_SURPLUS 删除多余证书
	//返回值: 0 得到一个完整数字证书链， -1 用户证书缺失， -2 不存在CA证书， > 0, 已组成证书链的证书数量(非完整数字证书链)
	int CheckCertificateChain( int flag );
	//验证对端的数字证书是否被吊销
	//参数：CRLFilePath CRL(Certificate Revocation Lists)文件本地路径
	//返回值：-1 操作失败，可能是文件不存在，或者CRL格式错误， 0 证书可用(未被吊销)，1 证书已被吊销
	int CheckCertIsRevoked(const char *CRLFilePath);
public:
	// 设置用于验证对端用户证书的CA证书
	// 参数: ca 证书文件路径,
	// 返回值: 0 数据读取成功，-1 证书文件异常
	int SetTrustCA( const char* ca );
private:
	struct Internal*	m_internal;
};

}
}
#endif // __INCLUDED_DAHUA_NETFRAMEWORK_SSL_X509_H__
