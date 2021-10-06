//
//  "$Id: SockAddr.h 48672 2012-02-03 03:13:11Z zhou_mingwei $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-5-7		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_SOCK_ADDR_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_SOCK_ADDR_H__

#include "Infra/IntTypes.h"
#ifdef WIN32
#include "NTypes_Win32.h"
#else
#include <sys/socket.h>
#endif
#include "NDefs.h"

namespace Dahua{
namespace NetFramework{

class NETFRAMEWORK_API CSockAddr{
public:
	CSockAddr();
	virtual ~CSockAddr();
public:
	//��ȡ����Ϊstruct sockaddr�ĵ�ַ�����ص�ָ�뼴Ϊ�������addr�ĵ�ַ��
	virtual struct sockaddr * GetAddr(  struct sockaddr * addr ) const = 0;
	//��ȡGetAddr��ȡ�ĵ�ַ�ĳ��ȡ�
	virtual uint32_t GetAddrLen() const = 0;

	//��ǰ��ַ�����ͣ��ݴ����Ϳ�����������������ת��
	uint32_t GetType() const;
	//��ǰ��ַ�Ƿ�Ϊ��Ч�ĵ�ַ����
	virtual bool IsValid()const = 0;
	//�ڴ󻪵ĸ����豸�У��ܶ����ʹ�ù̶�����Ķ˿ڡ���ͱ�ʾ���˵���1024��ϵͳ��
	//���˿ڣ����豸Ҳ������һ�������Ķ˿ڣ������ڱ�дӦ�õ�ʱ��ʹ�õ���ʱ�˿�Ҫ
	//ȷ��������Щ�����˿ڳ�ͻ������ӿ����ڲ�ѯ��ǰ�������ַ�Ƿ�Ϊ�ѱ����ĵ�ַ��
	//����ֵ��true��ʾ�ѱ�����false��ʾδ������
	bool IsReserved();
	/*SOCKADDR_TYPE_UNKNOWN		��Ч��ַ����
	  SOCKADDR_TYPE_IPV4		AF_INET���ַ����
	  SOCKADDR_TYPE_IPV6		AF_INET6���ַ����
	  SOCKADDR_TYPE_STORAGE		��ַ�洢���ͣ��ܴ洢AF_INET���AF_INET6���ַ����
	  SOCKADDR_TYPE_UN			AF_UNIX���ַ���ͣ��ṩ���ֻ��Ϊ�Ժ�ĵ�ַ������չ����ʾ������ǰ���ô���
	  */
	enum SOCKADDR_TYPE{ 
		SOCKADDR_TYPE_UNKNOWN, 
		SOCKADDR_TYPE_IPV4,
		SOCKADDR_TYPE_IPV6, 
		SOCKADDR_TYPE_STORAGE,
		SOCKADDR_TYPE_UN 
	};
protected:
	uint32_t m_type;
};

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_SOCK_ADDR_H__
