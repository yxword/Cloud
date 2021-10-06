//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_CLOUD_STORAGE_STRUCT_BASE_H__
#define __INCLUDE_DAHUA_CLOUD_STORAGE_STRUCT_BASE_H__

#include "Infra/IntTypes.h"
#include "Serialization.h"
#include <vector>
#include <string>	
#include <sstream>

namespace Dahua {
namespace EFS {

struct BlockIndex
{
	uint64_t fileId;
	uint16_t objectId;
	uint8_t blockId;
	BlockIndex();
	bool operator< ( const BlockIndex& rhs ) const;
	bool operator== ( const BlockIndex& rhs ) const;
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn);
	void print(std::ostringstream& oss) const;
};

struct Block
{
	struct BlockIndex blockIndex;
	uint64_t objOffset;
	uint32_t blockLength;
	Block();
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn);
	void print(std::ostringstream& oss) const;
};

struct DataNodeId
{
	std::string addr;
	int32_t port;
	std::string storageId; //"DS-randInt-ipaddr-currentTimeMillis"
	DataNodeId();
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn);
	void print(std::ostringstream& oss) const;
};

struct DatanodeRegistration
{
	std::string mdsDesc;
	struct DataNodeId dnId;
	DatanodeRegistration();
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn);
	void print(std::ostringstream& oss) const;
};

struct IBlockInfo
{
	uint8_t blockId;
	int8_t blockStatus;
	struct DataNodeId dnId;
	IBlockInfo();
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn);
	void print(std::ostringstream& oss) const;
};

struct LocatedObject
{
	uint64_t fileId;
	uint16_t objectId;
	uint8_t blockNum;
	int64_t offset;//object在文件中的位置
	int64_t size;
	int8_t status;//这个object的状态：0使用中,1已更新
	uint8_t dataNum;		//真实数据块数，即N
	uint8_t parityNum;	//冗余数据块数，即M
	uint8_t ECType;		//EC算法，0表示副本，1表示XOR
	int32_t pieceSize;	//分片大小，副本时无意义
	std::vector<struct IBlockInfo> iblockInfo;
	LocatedObject();
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn);
	void print(std::ostringstream& oss) const;
};

struct LocatedObjects
{
	int64_t fileLength;
	int8_t underConstruction;
	std::vector<struct LocatedObject> objs;
	LocatedObjects();
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn);
	void print(std::ostringstream& oss) const;
};


struct BlockTargetPair {
    std::vector<struct BlockIndex> blockIndexs;          // 待恢复blockIndex
    std::vector<struct DataNodeId> blockPlacesLocation;  // 恢复block到指定的datanode上
	struct LocatedObject object;                         // 恢复block所在的object信息
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn);
	void print(std::ostringstream& oss) const;
};

struct FileStatus{
	//文件基本属性
	int64_t			fileLength;
	uint32_t		modifyTime;
	uint32_t		createTime;
	uint8_t			isWriting;//是否正在被写入
	uint8_t			dataNum;
	uint8_t			parityNum;
	FileStatus();
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn);
	void print(std::ostringstream& oss) const;	
};

struct ShortFileStatus{
    std::string     fileName;
    int64_t         fileLength;
    uint32_t        createTime;
    uint32_t        modifyTime;
    ShortFileStatus();
    int length() const;
    bool Serialize( Serialization& sn ) const;
    bool Deserialize( Serialization& sn );
    void print( std::ostringstream& oss ) const;
};

struct DiskStatus{
	uint16_t slot;
	int16_t status;//0 exist,1 not exist,2 remove,3 add,...
	DiskStatus();
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn);
	void print(std::ostringstream& oss) const;
};

struct ErasureMapping
{
	uint16_t data;
	uint16_t parity;
	int16_t	 ECType;
	ErasureMapping();
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn);
	void print(std::ostringstream& oss) const;
};

struct UniqCollectionId
{
	uint64_t bucketId;
	uint64_t fileId;
	UniqCollectionId();
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn) ;
	void print(std::ostringstream& oss) const;
};

// 此结构体用于将文件基本信息存储到block上，以便于在发生极端异常时，从
// block上将元数据进行部分恢复。此结构体序列化之后作为value，key则使用、
// 字符串File_x,大文件必然为File_0,小文件根据在container里面的顺序，依次
// 为File_0,File_1,File_2...
struct FileDetail 
{
	uint8_t type;                 // 文件类型，大文件1,小文件2
	uint64_t bucketId;            // 
    uint64_t fileId;			  // 对于小文件，是container id
    uint64_t offset;              // 文件在这个对象内的偏移，大文件为0，小文件视每个文件情况而定
    uint64_t size;                // 文件长度，小文件才有意义，大文件固定为0，以object真实长度为准
    std::string bucketName;      // 
    std::string fileName;        // 
	FileDetail();
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn) ;
	void print(std::ostringstream& oss) const;
};

struct DataNodeMirrorDetail
{
	std::string uid;
	int64_t totalBlockNum;
	int64_t toAddBlockNum;
	int64_t toReduceBlockNum;
	DataNodeMirrorDetail();
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn);
	void print(std::ostringstream& oss) const;
};

} // namespace CloudStorage
} // namespace Dahua
#endif //__INCLUDE_DAHUA_CLOUD_STORAGE_STRUCT_BASE_H__

