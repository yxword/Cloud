//
//  "$Id: Message.h 7672 2010-5-10 02:28:18Z he_linqiang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-6-2        he_linqiang Create
//

#ifndef __INCLUDED_PRIVATE_POOL_H__
#define __INCLUDED_PRIVATE_POOL_H__

#include "NetFramework/NetHandler.h"
using namespace Dahua::NetFramework;
#define MSG_QUERY		0x1
#define MSG_REPORT	0x2

class CMyHandler;

class CPool
{
public:
	CPool() {}
	~CPool() {}
public:
	void Start();
private:
	CMyHandler* my_handler;
};

class CMyHandler1;
class CMyHandler : public CNetHandler
{
public:
	CMyHandler() {}
	virtual ~CMyHandler() {}
public:
	int handle_message( long from_id, MSG_TYPE type, long attach );
	int StartQuery(int idx);
private:
	CMyHandler1 *m_handle1[10];
};

class CMyHandler1 : public CNetHandler
{
public:
	CMyHandler1( int pool_idx, int id );
	~CMyHandler1() {}
public:
	int handle_message( long from_id, MSG_TYPE type, long attach );
	int handle_close( CNetHandler* myself );
public:
	int	m_id;	
};

int StartPrivatePoolTest();

#endif //__INCLUDED_MESSAGE_H__
