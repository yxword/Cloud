//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_BEING_MASTER_STATE_H__
#define __INCLUDE_DAHUA_EFS_BEING_MASTER_STATE_H__

#include "Json/json.h"
#include "Common/Defs.h"
#include "Infra/Mutex.h"

namespace Dahua {
namespace EFS {

class CBeingMasterState
{
public:
	static CBeingMasterState* instance();
	bool init(const std::string& file, const std::string& prefix);

	int32_t getHAState();
	int32_t getHABecomeMasterTime();
	bool setHAState(int state);
	bool setHABecomeMasterTime(int32_t time);

private:
	CBeingMasterState();
	~CBeingMasterState();

private:
    class Internal;
    class Internal* m_internal;
};

#define gBeingMasterState CBeingMasterState::instance()

}
}


#endif /* __INCLUDE_DAHUA_EFS_BEING_MASTER_STATE_H__ */
