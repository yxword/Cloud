//
//  "$Id: AsyncMsg.h 31672 2016-03-18 09:03:00 wang_zhihao $"
//
//  Copyright (c)1992-2016, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2016-3-18		wang_zhihao	Create
//

#ifndef __INCLUDED_ASYNC_MSG_H__
#define __INCLUDED_ASYNC_MSG_H__

#include "NetFramework/NetHandler.h"
#include "Infra/Thread.h"

#include <iostream>

using namespace Dahua::NetFramework;

#define MSG_REQ  1
#define MSG_RESP 2

class CMsgEntry : public CNetHandler
{
public:
     CMsgEntry( CMsgEntry* receiveEntry = NULL)
        : m_receive_entry(receiveEntry)
     {
         if (m_receive_entry != NULL)
        	 SetDefaultRcer(m_receive_entry->GetID());
     }

     virtual ~CMsgEntry()
     {
     }

     void start()
     {
         SetTimer( 1000000 );
     }

     int handle_message( long from_id, MSG_TYPE type, long attach )
     {
        switch( type )
		{
			case MSG_REQ:
				std::cout << "Msg from id " << from_id
					  	  << ", type "      << type
					  	  << ", thread id " << Dahua::Infra::CThread::getCurrentThreadID()
				  	  	  << std::endl;
				Notify( from_id, MSG_RESP, attach);
				break;
			case MSG_RESP:
				std::cout << "Msg from id " << from_id
					  	  << ", type "      << type
					  	  << ", thread id " << Dahua::Infra::CThread::getCurrentThreadID()
				  	  	  << std::endl;
				break;
			default:
				std::cerr << "Msg type error!" << std::endl;
				break;
		}
	
        return 0;
     }

     int64_t handle_timeout( long id )
     {
         Notify( MSG_REQ, 0);
         //notify myself
         //Notify( GetID(), MSG_REQ, 0);

         return 0;
     }

     int handle_close( CNetHandler* myself )
     {
         std::cout << "NetHandler close, thread id " << Dahua::Infra::CThread::getCurrentThreadID() << std::endl;

         return 0;
     }

private:
     CMsgEntry* m_receive_entry;
};

#endif //__INCLUDED_ASYNC_MSG_H__
