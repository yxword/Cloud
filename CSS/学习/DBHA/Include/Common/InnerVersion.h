//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#include <string.h>
#include <iostream>
#include "Serialization.h"

#ifndef __INCLUDE_DAHUA_EFS_INNERVERSION_H__
#define __INCLUDE_DAHUA_EFS_INNERVERSION_H__

namespace Dahua {
namespace EFS {

// 内部使用的版本号
class CInnerVersion
{
public:
    CInnerVersion();
    CInnerVersion(int majorNum, int minorNum, int revisionNum, int buildNum);
	CInnerVersion(const CInnerVersion& obj);
	CInnerVersion& operator=(const CInnerVersion& obj);
	
    friend bool operator ==(const CInnerVersion & lhs, const CInnerVersion & rhs);
    friend bool operator !=(const CInnerVersion & lhs, const CInnerVersion & rhs);
    friend bool operator <(const CInnerVersion & lhs, const CInnerVersion & rhs);
    friend bool operator>( const CInnerVersion& lhs, const CInnerVersion& rhs );
    friend bool operator<=( const CInnerVersion& lhs, const CInnerVersion& rhs );
    friend bool operator>=( const CInnerVersion& lhs, const CInnerVersion& rhs );
    friend std::ostream & operator <<(std::ostream & out, const CInnerVersion & iv);

    int getBuildNum() const;
    int getMajorNum() const;
    int getMinorNum() const;
    int getRevisionNum() const;
    void setBuildNum(int buildNum);
    void setMajorNum(int majorNum);
    void setMinorNum(int minorNum);
    void setRevisionNum(int revisionNum);
    int length();
    bool serialize(Serialization& sn);
    bool deserialize(Serialization& sn);
private:
    int m_major_num;    // 主版本号，表示架构等的大修改。
    int m_minor_num;    // 副版本号，表示xxx。
    int m_revision_num; // 修正版本号，表示bug修复、代码优化。
    int m_build_num;    // 编译版本号，暂使用svn号。
};

} //EFS
} //Dahua
#endif /* __INCLUDE_DAHUA_EFS_MDS_INNERVERSION_H__ */
