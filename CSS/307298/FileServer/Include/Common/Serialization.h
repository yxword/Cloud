//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __INCLUDE_DAHUA_EFS_COMMON_SERIALIZATION_H__
#define __INCLUDE_DAHUA_EFS_COMMON_SERIALIZATION_H__

#include <assert.h>
#include <string>
#include <vector>
#include <string.h>
#include "Infra/IntTypes.h"

namespace Dahua {
namespace EFS {

#define EFS_SUCCESS 0
#define EFS_ERROR -1

#define INT8_SIZE ((uint32_t)sizeof(int8_t))
#define INT16_SIZE ((uint32_t)sizeof(int16_t))
#define INT32_SIZE ((uint32_t)sizeof(int32_t))
#define INT64_SIZE ((uint32_t)sizeof(int64_t))

inline uint32_t sizeOfString( const std::string& s )
{
	return s.empty() ? INT32_SIZE : INT32_SIZE + 1 + s.size();
}

inline uint32_t sizeOfPointer(const char* buffer, uint32_t bufLength)
{
	return buffer != NULL ?  INT32_SIZE + bufLength + 1 : INT32_SIZE;
}

#define VECTOR_STRUCT_LENGTH(DO_LENGTH) do { \
	int length = INT32_SIZE; \
	for(size_t i = 0; i < vec.size(); ++i){ \
		length += DO_LENGTH; } \
	return length; \
} while(0)

#define VECTOR_BASE_TYPE_LENGTH(TYPESIZE) do { \
	int length = INT32_SIZE; \
	length += vec.size() * TYPESIZE; \
	return length; \
} while(0)


template <typename T>
uint32_t sizeOfVector(const std::vector<T>& vec) { VECTOR_STRUCT_LENGTH(vec.at(i).length()); }
template <>
inline uint32_t sizeOfVector<int8_t>(const std::vector<int8_t>& vec) { VECTOR_BASE_TYPE_LENGTH(INT8_SIZE); } 
template <>
inline uint32_t sizeOfVector(const std::vector<uint8_t>& vec) { VECTOR_BASE_TYPE_LENGTH(INT8_SIZE); } 
template <>
inline uint32_t sizeOfVector(const std::vector<int16_t>& vec) { VECTOR_BASE_TYPE_LENGTH(INT16_SIZE); } 
template <>
inline uint32_t sizeOfVector(const std::vector<uint16_t>& vec) { VECTOR_BASE_TYPE_LENGTH(INT16_SIZE); } 
template <>
inline uint32_t sizeOfVector(const std::vector<int32_t>& vec) { VECTOR_BASE_TYPE_LENGTH(INT32_SIZE); } 
template <>
inline uint32_t sizeOfVector(const std::vector<uint32_t>& vec) { VECTOR_BASE_TYPE_LENGTH(INT32_SIZE); } 
template <>
inline uint32_t sizeOfVector(const std::vector<int64_t>& vec) { VECTOR_BASE_TYPE_LENGTH(INT64_SIZE); } 
template <>
inline uint32_t sizeOfVector(const std::vector<uint64_t>& vec) { VECTOR_BASE_TYPE_LENGTH(INT64_SIZE); } 
template <>
inline uint32_t sizeOfVector(const std::vector<std::string>& vec) { VECTOR_STRUCT_LENGTH(sizeOfString(vec.at(i))); } 

//desc: Serialization?????????????????????????????????????????????

class Serialization
{
public:
	Serialization();
	~Serialization();

	/// ??????????????????????????????????????????????????????????????????????????????????????????
	/// \param data ????????????????????????
	///	\param len ????????????????????????
	void setDeserialize(const char* data,const int64_t dataLen);
	
	/// ??????????????????????????????
	bool isDeserializeComplete();

	/// ???????????????????????????
	/// \param dataLen ????????????
	int setSerialize(const int64_t dataLen);

	/// ??????????????????????????????
	/// \param len ????????????????????????
	const char* getSerialize(int32_t &len);
	
	/// ?????????????????????????????????
	const char* getSerializeLastPointer();

	/// ????????????
	void reset();
	
	/// ???????????????????????????????????????
	int readInt8(int8_t* value);

	/// ??????2??????????????????????????????
	int readInt16(int16_t* value);

	/// ??????4??????????????????????????????
	int readInt32(int32_t* value);

	/// ??????8??????????????????????????????
	int readInt64(int64_t* value);

	/// ??????????????????????????????????????????
	int readString(char* str,int64_t strLen,int64_t& realLen);

	/// ??????????????????????????????????????????
	int readString(std::string& str);

	/// ??????????????????????????????????????????
	int readData(const char **dataptr,int64_t& len);
	
	/// ?????????????????????????????????,?????????????????????string??????vector?????????
	int readBytes(void* buf,int64_t bufLen);

	/// ?????????1?????????
	int writeInt8(const int8_t value);

	/// ?????????2?????????
	int writeInt16(const int16_t value);

	/// ?????????4?????????
	int writeInt32(const int32_t value);

	/// ?????????8?????????
	int writeInt64(const int64_t value);

	/// ????????????????????????
	int writeString(const char* str,int strLen);

	/// ????????????????????????
	int writeString(const std::string& str);

	/// ????????????????????????
	int writeBytes(const void* buf,int64_t bufLen);

	/// ?????????vector
	template <typename T>
	int writeVector(const std::vector<T>& vec);
	/// ????????????vector
	template <typename T>
	int readVector(std::vector<T>& vec);
private:
	struct Internal;
	struct Internal*	m_internal;
};

//??????vector??????????????????
template <typename T>
int Serialization::writeVector(const std::vector<T>& vec)
{
	int ret = EFS_SUCCESS;
	int length = (int)vec.size();
	ret = writeInt32(length);
	if( EFS_SUCCESS != ret ){
		return ret;
	}

	for(size_t i = 0; i < vec.size(); ++i){
		if(!vec.at(i).Serialize(*this)){
			ret = EFS_ERROR;
			break;
		}
	}

	return ret;
}

template <typename T>
int Serialization::readVector(std::vector<T>& vec)
{
	int ret = EFS_SUCCESS;
	int length = 0;
	ret = readInt32(&length);
	if( EFS_SUCCESS != ret ){
		return ret;
	}

	for(int i = 0; i < length; ++i){
		T t;
		if(!t.Deserialize(*this)){
			ret = EFS_ERROR;
			break;
		}
		vec.push_back(t);
	}
	return ret;
}

//??????vector???????????????
#define SERIALIZE_VECTOR(WRITE_FUNC) do { \
	int ret = EFS_SUCCESS;  \
	int length = (int)vec.size(); \
	ret = writeInt32(length); \
	if( EFS_SUCCESS != ret ){ \
		return ret; \
	} \
	for(int i = 0; i < length; ++i){ \
		ret = WRITE_FUNC(vec.at(i)); \
		if( EFS_SUCCESS != ret ){ \
			break; \
		} \
	} \
	return ret; \
} while(0) 

//DO_READ ?????????????????????????????????
//TYPE ????????????
//DO_PUSH_VECTOR ????????????vector
#define DESERIALIZE_BASE_VECTOR(DO_READ,TYPE,DO_PUSH_VECTOR) do { \
	int ret = EFS_SUCCESS; \
	int32_t length = 0; \
	ret = readInt32(&length); \
	if( EFS_SUCCESS != ret ){ \
		return ret; \
	} \
	for(int32_t i = 0; i < length; ++i){ \
		TYPE item; \
		DO_READ; \
		if( EFS_SUCCESS != ret ){ \
			break; \
		} \
		DO_PUSH_VECTOR; \
	} \
	return ret;\
} while(0) 

//????????????
#define PUSH_VECTOR vec.push_back(item)  
//??????????????????????????????
#define PUSH_IVECTOR(TYPE,ISUNSIGNED) if(ISUNSIGNED){ vec.push_back((u##TYPE)(item)); } else { vec.push_back(item); }
//????????????int?????????vector
#define DESERIALIZE_INT_VECTOR(READ_FUNC,TYPE,ISUNSIGNED) DESERIALIZE_BASE_VECTOR((ret = READ_FUNC(&item)),TYPE,PUSH_IVECTOR(TYPE,ISUNSIGNED))
//????????????string?????????vector
#define DESERIALIZE_STRING_VECTOR(READ_FUNC,TYPE) DESERIALIZE_BASE_VECTOR((ret = READ_FUNC(item)),TYPE,PUSH_VECTOR)

template <>
inline int Serialization::writeVector(const std::vector<int8_t>& vec) { SERIALIZE_VECTOR(writeInt8); }
template <>
inline int Serialization::writeVector(const std::vector<uint8_t>& vec) { SERIALIZE_VECTOR(writeInt8); }
template <>
inline int Serialization::writeVector(const std::vector<int16_t>& vec) { SERIALIZE_VECTOR(writeInt16); }
template <>
inline int Serialization::writeVector(const std::vector<uint16_t>& vec) { SERIALIZE_VECTOR(writeInt16); }
template <>
inline int Serialization::writeVector(const std::vector<int32_t>& vec) { SERIALIZE_VECTOR(writeInt32); }
template <>
inline int Serialization::writeVector(const std::vector<uint32_t>& vec) { SERIALIZE_VECTOR(writeInt32); }
template <>
inline int Serialization::writeVector(const std::vector<int64_t>& vec) { SERIALIZE_VECTOR(writeInt64); }
template <>
inline int Serialization::writeVector(const std::vector<uint64_t>& vec) { SERIALIZE_VECTOR(writeInt64); }
template <>
inline int Serialization::writeVector(const std::vector<std::string>& vec) { SERIALIZE_VECTOR(writeString); }

template <>
inline int Serialization::readVector(std::vector<int8_t>& vec) { DESERIALIZE_INT_VECTOR(readInt8,int8_t,false); }
template <>
inline int Serialization::readVector(std::vector<uint8_t>& vec) { DESERIALIZE_INT_VECTOR(readInt8,int8_t,true); }
template <>
inline int Serialization::readVector(std::vector<int16_t>& vec) { DESERIALIZE_INT_VECTOR(readInt16,int16_t,false); }
template <>
inline int Serialization::readVector(std::vector<uint16_t>& vec) { DESERIALIZE_INT_VECTOR(readInt16,int16_t,true); }
template <>
inline int Serialization::readVector(std::vector<int32_t>& vec) { DESERIALIZE_INT_VECTOR(readInt32,int32_t,false); }
template <>
inline int Serialization::readVector(std::vector<uint32_t>& vec) { DESERIALIZE_INT_VECTOR(readInt32,int32_t,true); }
template <>
inline int Serialization::readVector(std::vector<int64_t>& vec) { DESERIALIZE_INT_VECTOR(readInt64,int64_t,false); }
template <>
inline int Serialization::readVector(std::vector<uint64_t>& vec) { DESERIALIZE_INT_VECTOR(readInt64,int64_t,true); }
template <>
inline int Serialization::readVector(std::vector<std::string>& vec) { DESERIALIZE_STRING_VECTOR(readString,std::string); }

} //end namespace cloudStorage
} //end namespace dahua

#endif //__INCLUDE_DAHUA_EFS_COMMON_SERIALIZATION_H__
