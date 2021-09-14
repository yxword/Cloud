//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_BLOCK_COMMAND_H__
#define __INCLUDE_DAHUA_EFS_BLOCK_COMMAND_H__

#include "DatanodeCommand.h"
#include "Infra/IntTypes.h"
#include "Common/StructBase.h"
#include "Common/Serialization.h"
#include <sstream>
#include <vector>

namespace Dahua {
namespace EFS {

class CMds2dnDatanodeCommandMessage;
class CBlockCommand : public CDatanodeCommand
{
public:
	//空的command,action为unknown
	CBlockCommand();
	//初始化设置action,带block index数组
	CBlockCommand( int action, std::vector<struct BlockIndex>& blockIndexs );
	//初始化设置action,带block target列表
	CBlockCommand( int action, std::vector<struct BlockTargetPair>& blockTargetList );
	//析构
	~CBlockCommand();

	//获取block index列表
	void getBlockIndexs( std::vector<struct BlockIndex>& blockIndexs );
	//获取target列表
	void getTargets( std::vector<struct BlockTargetPair>& targets );

protected:
	friend class CMds2dnDatanodeCommandMessage;
	virtual int level();
	virtual int length();
	virtual bool serialize( Serialization& sn );
	virtual bool deserialize( Serialization& sn );
	virtual void print( std::ostringstream& oss ) const;

protected:
	struct Internel{
		std::vector<struct BlockIndex> m_block_indexs;
		std::vector<struct BlockTargetPair> m_targets;
	};

	struct Internel* m_internel;
};

} //namespace EFS
} //namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_BLOCK_COMMAND_H__

