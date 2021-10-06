//
//  "$Id: NTypes_Win32.h 30245 2011-07-20 05:26:11Z wang_haifeng $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					
//

#ifndef __INCLUDE_DAHUA_NETFRAMEWORK_NTYPES_WIN32_H__
#define __INCLUDE_DAHUA_NETFRAMEWORK_NTYPES_WIN32_H__	

#include "Winsock2.h"
#include "Ws2tcpip.h"
#include "Windows.h"
#include <stdio.h>

struct iovec
{
	unsigned long iov_len;
	void  *iov_base;	
};

#endif //__INCLUDE_DAHUA_NETFRAMEWORK_NTYPES_WIN32_H__
