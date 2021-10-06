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

#ifndef __INCLUDED_MESSAGE_H__
#define __INCLUDED_MESSAGE_H__

#include "NetFramework/NetHandler.h"
using namespace Dahua::NetFramework;
#define MSG_QUERY		0x1
#define MSG_REPORT	0x2

class CReport;
class CQuery : public CNetHandler
{
public:
	CQuery() {}
	virtual ~CQuery() {}
public:
	int handle_message( long from_id, MSG_TYPE type, long attach );
	int StartQuery();
private:
	CReport *m_report[10];
};

class CReport : public CNetHandler
{
public:
	CReport( int id ) { m_id = id; }
	~CReport() {}
public:
	int handle_message( long from_id, MSG_TYPE type, long attach );
	int handle_close( CNetHandler* myself );
public:
	int	m_id;	
};

int StartMessageTest();

#endif //__INCLUDED_MESSAGE_H__
