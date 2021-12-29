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
	std::string storageId; // 原设计是"DS-randInt-ipaddr-currentTimeMillis"，实际值和dnindex相同，就是一个数字表示
	std::string uuid;	   // DN自身生成的唯一UUID，元数据集群每组MDS看到的都是一样的，固定长度32字符
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
	std::string dnVer;
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
	int32_t blockSize;      // EFS 7.0 支持Append新增. see redmine #16045
	int32_t dnIndex;        // EFS 7.0 支持Append新增.dn在mds上的id,直接用IP的话,在网闸模式下不利于MDS区分dn. see redmine #16045
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
	int64_t size; //object的真实大小，不是默认的大小
	int8_t status;//这个object的状态：0使用中(size为0，offset为-1),1已更新
	uint8_t dataNum;		//真实数据块数，即N
	uint8_t parityNum;	//冗余数据块数，即M
	uint8_t ECType;		//EC算法，0表示副本，1表示XOR
	int32_t pieceSize;	//分片大小，副本时无意义
	std::vector<struct IBlockInfo> iblockInfo;
	int32_t defaultBlockSize; //EFS支持的Block默认大小
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

// 恢复和迁移共用该结构
struct BlockTargetPair {
    std::vector<struct BlockIndex> blockIndexs;          // 待恢复/迁移block
    std::vector<struct DataNodeId> blockPlacesLocation;  // 待恢复/迁移block的目标dn
	struct LocatedObject object;                         // 待恢复block所在的object信息
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
	uint32_t		lockStat;
private:
    char              *fileAttribute;
    uint32_t        fileAttrLength;
public:
    ShortFileStatus();
    ShortFileStatus(const ShortFileStatus &fileStatus);
    ~ShortFileStatus();
    void setFileAttribute( const char *fileAttributeContent , uint32_t fileAttributeLength);
    const char * getFileAttribute();
    uint32_t getFileAttributeLength();
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
    std:: string bucketName;      //
    std:: string fileName;        //
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

struct StorageUser
{
	uint32_t id;
	std::string userName;
	std::string nickName;
	uint64_t createTime;
	uint64_t capacity;  //总容量，例如 10TB
	uint64_t usedCapacity;  //已经使用容量，例如 5TB，其中有一个bucket设置了配额2TB，使用了1TB
	uint64_t reservedCapacity; //已分配容量，例如 2TB(已分配配额)+(5T-1T)(未分配配额的bucket总使用量)
	uint32_t bucketLimitNum;
	uint32_t bucketCount;
	uint32_t state;
	uint8_t  privilege;

	uint64_t remainCapacity;  //该用户可用的剩余容量
	uint8_t  enableRecycle;   //用户使能生命周期标记
	uint8_t  recycleAction;   //用户使能生命周期后的覆盖行为:等比例或等时间删除
	
    std::string passwd;

    uint32_t authId;	// 统一鉴权ID
	
	StorageUser();
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn) ;
	void print(std::ostringstream& oss) const;
};

struct RegionInfo
{
	uint16_t    port;
	std::string name;
	std::string nickName;
	std::string vip;

	RegionInfo();
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn) ;
	void print(std::ostringstream& oss) const;
};

struct BucketLifeCycleInfo
{
	std::string bucketName; 			//bucket名
	std::string prefixName; 			//设置目录前缀，目前保留未使用
	uint64_t bucketID;      			//bucketID
	int64_t  delay;          			//设置该bucket的文件保留时间，以秒为单位
	uint64_t bucketCTime;               //bucket的创建时间
	uint8_t  enableAction;				//设置bucket是否加入用户紧急覆盖,true=1为使能,false=0为去使能
	uint8_t  removeWhenBucketEmpty;		//空bucket是否删除标记
	uint8_t  action;         			//设置bucket循环覆盖触发后的动作
	uint64_t capacity;                  // bucket配额容量
	uint32_t userId;                    // 用户ID

	BucketLifeCycleInfo();
	int length() const;
	bool Serialize(Serialization& sn) const;
	bool Deserialize(Serialization& sn) ;
	void print(std::ostringstream& oss) const;
};

} // namespace CloudStorage
} // namespace Dahua
#endif //__INCLUDE_DAHUA_CLOUD_STORAGE_STRUCT_BASE_H__

