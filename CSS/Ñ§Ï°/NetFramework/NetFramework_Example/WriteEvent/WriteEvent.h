//
//  "$Id: WriteEvent.h 7672 2010-6-28 02:28:18Z he_linqiang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-6-28        he_linqiang Create
//
//
#ifndef __INCLUDED_WRITE_EVENT_H__
#define __INCLUDED_WRITE_EVENT_H__

#include "NetFramework/NetHandler.h"
#include "NetFramework/SockAddrIPv4.h"
#include "NetFramework/SockStream.h"
using namespace Dahua::NetFramework;
class CWriteEvent : public CNetHandler
{
public:
	CWriteEvent();
	virtual ~CWriteEvent();
public:
	int StartSession( CSockAddr & addr );
	virtual int64_t handle_timeout( long handle );
	virtual int handle_output( int handle );
	virtual int handle_output_timeout( int handle );
	//要检测socket上的异常，要实现该函数。
	virtual int handle_exception( int handle );
	
	virtual int handle_close(CNetHandler *myself);
private:
	int do_send();
private:
	CSockStream	m_stream;
	int 				m_status;
};

int StartWriteEvent();

#endif //__INCLUDED_WRITE_EVENT_H__
