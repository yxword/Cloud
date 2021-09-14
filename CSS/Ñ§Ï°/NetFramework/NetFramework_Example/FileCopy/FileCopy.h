//
//  "$Id: FileCopy.h 7672 2010-5-17 02:28:18Z he_linqiang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2011-4-28        he_linqiang Create
//
#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_FILE_COPY_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_FILE_COPY_H__

#include "NetFramework/NetHandler.h"
#include "Infra/Mutex.h"
using namespace Dahua::NetFramework;
using namespace Dahua::Infra;
namespace Dahua{
	namespace NetFramework{
		class CNFile;
	}
}

class CFileCopier : public CNetHandler
{
public:
	CFileCopier();
	virtual ~CFileCopier();
public:
	int StartCopy();
	int handle_message( long from_id, MSG_TYPE type, long attach );
	virtual int handle_close( CNetHandler* myself );
private:
	bool copy_read();
	bool copy_write();
private:
	CNFile*		m_file_write;
	CNFile*		m_file_read;
	char*			m_rw_buf;
	int				m_read_len;
	int				m_write_len;
	CMutex		m_copy_mutex;
};

int StartFileCopyTest();

#endif //__INCLUDED_DAHUA_NETFRAMEWORK_FILE_COPY_H__
