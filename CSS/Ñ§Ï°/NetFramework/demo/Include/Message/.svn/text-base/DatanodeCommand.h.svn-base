//
//  "$Id$"
//
//  Copyright ( c )1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __INCLUDE_DAHUA_EFS_DATANODE_COMMAND_H__
#define __INCLUDE_DAHUA_EFS_DATANODE_COMMAND_H__

#include "Infra/IntTypes.h"
#include "Common/Serialization.h"
#include <sstream>

namespace Dahua {
namespace EFS {

enum{
	DNA_NULL,					//空回复
	DNA_UNKNOWN,				//未知命令
	DNA_REGISTER,				//当mds不存在datanode信息则任何消息到mds，mds都会下发要求dn注册
	DNA_INVALIDATE,				//无效块，datanode接收到后删除无效块		
	DNA_SHUTDOWN,				//关闭datanode
	DNA_RECOVERBLOCK,			//块丢失或异常,从其他节点恢复回来
	DNA_REPORTBLOCK 			//通知datanode从什么位置开始块汇报
};

class CMds2dnDatanodeCommandMessage;
class CDatanodeCommand
{
public:
	//默认ation为unknown
	CDatanodeCommand();
	//根据指定action进行设置
	CDatanodeCommand( int action );
	//虚析构
	virtual ~CDatanodeCommand();

	//获取action
	int getAction() const;
	//设置附加数据
	void setExtra( int64_t data );
	//获取附加数据
	int64_t getExtra() const;

#ifndef WIN32	
protected:
	friend class CMds2dnDatanodeCommandMessage;
#endif
	virtual int level();
	virtual int length();
	virtual bool serialize( Serialization& sn );
	virtual bool deserialize( Serialization& sn );
	virtual void print( std::ostringstream& oss ) const;

protected:
	struct Internel{
		int m_level;
		int m_action;
		int64_t m_extra_data;
	};

	struct Internel* m_internel;
};

} //namespace EFS
} //namespace Dahua

#endif //__INCLUDE_DAHUA_EFS_DATANODE_COMMAND_H__
