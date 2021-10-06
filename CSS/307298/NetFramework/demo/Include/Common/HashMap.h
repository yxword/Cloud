//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef WIN32
#ifndef __INCLUDE_DAHUA_CLOUD_EFS_HASH_MAP_H__
#define __INCLUDE_DAHUA_CLOUD_EFS_HASH_MAP_H__

#include <stdio.h>
#include <assert.h>
#include <algorithm>
#include "Infra/IntTypes.h"
#include "Memory/SharedPtr.h"
#include "dict.h"

namespace Dahua {
namespace EFS {

typedef struct HashMapFuncs
{
    // 计算键的哈希值函数
    typedef unsigned int (*HashFunc)(const void *key);
    // 复制键的函数
    // 如果传递进来的是key是栈变量，则需要设置keydup，使得内部key变成堆上变量
    // 如果传递进来的是key已经是堆上的变量，则可以不用设置keydup
    typedef void* (*KeyDupFunc)(void *privdata, const void *key);
    // 复制值的函数
    // 如果传递进来的是val是栈变量，则需要设置valdup，使得内部key变成堆上变量
    // 如果传递进来的是val已经是堆上的变量，则可以不用设置valdup
    typedef void* (*ValDupFunc)(void *privdata, const void *obj);
    // 对比两个键的函数, 返回大于0表示相等，否则表示不等
    // 对比函数必须得有否则程序会崩溃
    typedef int (*KeyCmpFunc)(void *privdata, const void *key1, const void *key2);
    // 键的释构函数，因为key是在堆上，如果不释放，则可能会有内存泄露
    typedef void (*KeyReleaseFunc)(void *privdata, void *key);
    // 值的释构函数，因为val是在堆上，如果不释放，则可能会有内存泄露
    typedef void (*ValReleaseFunc)(void *privdata, void *obj);

	HashFunc hash;
	KeyDupFunc keydup;
	ValDupFunc valdup;
	//key 比较函数必须有
	KeyCmpFunc keycmp;
	KeyReleaseFunc keyrelease;
	ValReleaseFunc valrelease;
    
	/*注释掉构造函数，避免和外部定义时初始化产生冲突
	HashMapFuncs()
    {
    	hash = NULL;
    	keydup = NULL;
    	valdup = NULL;
    	keycmp = NULL;
    	keyrelease = NULL;
    	valrelease = NULL;
    }
	*/

}HashMapFuncs;

//默认提供的三个hash函数
unsigned int IntHashFunction(unsigned int key);
unsigned int GenHashFunction(const void *key, int len);
unsigned int GenCaseHashFunction(const unsigned char *buf, int len);

typedef struct __IteratorData{
	__IteratorData()
	{
		iterator = NULL;
		entry = NULL;
	}
	
	~__IteratorData()
	{
		if(iterator != NULL) {
			dictReleaseIterator((dictIterator*)iterator);
			iterator = NULL;
		}
		entry = NULL;
	}

	void* iterator;
	void* entry;
}__IteratorData;

template<typename K,typename V>
struct __Iterator{
	__Iterator();
	__Iterator( const __Iterator& rhs );
	__Iterator& operator= ( const __Iterator& rhs );
	~__Iterator();
	//获取迭代器中的key
	K* first();
	//获取迭代器中的value
	V* second();

	Memory::TSharedPtr<__IteratorData>  d;
};
	
template<typename K,typename V>
class CHashMap {
public:	
	typedef struct __Iterator<K,V> Iterator;
public:
	explicit CHashMap(void* pridata,HashMapFuncs& funcs);
	~CHashMap();

	//获取值,不存在则返回NULL
	V* get(const K* k);
	
	//键不存在,则插入,返回 1;键存在则覆盖记录,返回0
	int put(K* k,V* v);

	//键不存在,则插入,返回 true;键存在则失败,返回false
	bool insert(K* k,V* v);

	//获取迭代器,总是指向begin	
	Iterator getIterator();

	//还有值，返回true,否则返回false
	bool next(Iterator & iterator);

	//获取map的长度
	uint32_t size();
	
	//map是否没有元素，空返回true,否则返回false
	bool empty();
	
	//清空map
	void clear();
	
	//删除一个k所对应的kv对，value可以选择是否要释放
	bool erase(const K* k,bool needFree=true);
	
	void swap(CHashMap<K,V>& rhs);
private:
	CHashMap();
	CHashMap(const CHashMap& rhs);
	CHashMap& operator=(const CHashMap& rhs);	
private:
	class Internel;
	Internel* m_internel;
};

  
template<typename K,typename V>
__Iterator<K,V>::__Iterator()
:d(new __IteratorData)
{
}

template<typename K,typename V>
__Iterator<K,V>::__Iterator( const __Iterator& rhs )
{
	d = rhs.d;
}

template<typename K,typename V>
__Iterator<K,V>& __Iterator<K,V>::operator= ( const __Iterator& rhs )
{
	d = rhs.d;
	return *this;
}

template<typename K,typename V>
__Iterator<K,V>::~__Iterator()
{
	//if( d.unique() && NULL != d->iterator){
	//	dictReleaseIterator((dictIterator*)d->iterator);
	//}
}

template<typename K,typename V>
K* __Iterator<K,V>::first()
{
	if(NULL == d->entry) return NULL;
	void* k = ((dictEntry*)d->entry)->key;
	//return static_cast<K*>(k);
	return (K*)k;
}

template<typename K,typename V>
V* __Iterator<K,V>::second()
{	
	if(NULL == d->entry) return NULL;
	void* v = ((dictEntry*)d->entry)->v.val;
	return static_cast<V*>(v);
}

template<typename K,typename V>
class CHashMap<K,V>::Internel
{
public:
	void* m_pridata;
	dict* m_dict;
	dictType* m_dict_type;

	HashMapFuncs m_hash_funcs;

	Internel()
	{
		m_pridata = NULL;
		m_dict = NULL;
		m_dict_type = NULL;
	}

	~Internel()
	{
		if( NULL != m_dict ){
			dictRelease(m_dict);
			m_dict = NULL;
		}

		if( NULL != m_dict_type ){
			delete m_dict_type;
			m_dict_type = NULL;
		}
	}
};

template<typename K,typename V>
CHashMap<K,V>::CHashMap( void *pridata ,HashMapFuncs& funcs)
{
	m_internel = new Internel;
	assert(NULL != m_internel );

	m_internel->m_pridata = pridata;
	m_internel->m_hash_funcs = funcs;
	
	assert(NULL != m_internel->m_hash_funcs.keycmp);

	m_internel->m_dict_type = new dictType;
	assert( NULL != m_internel->m_dict_type );

	m_internel->m_dict_type->hashFunction = m_internel->m_hash_funcs.hash;
	m_internel->m_dict_type->keyDup = m_internel->m_hash_funcs.keydup;
	m_internel->m_dict_type->valDup = m_internel->m_hash_funcs.valdup;
	m_internel->m_dict_type->keyCompare = m_internel->m_hash_funcs.keycmp;
	m_internel->m_dict_type->keyDestructor = m_internel->m_hash_funcs.keyrelease;
	m_internel->m_dict_type->valDestructor = m_internel->m_hash_funcs.valrelease;

	m_internel->m_dict = dictCreate(m_internel->m_dict_type,m_internel->m_pridata);
	
	assert(NULL != m_internel->m_dict);
}

template<typename K,typename V>
CHashMap<K,V>::~CHashMap()
{
	if(NULL != m_internel){
		delete m_internel;
		m_internel = NULL;
	}
}

template<typename K,typename V>
V* CHashMap<K,V>::get(const K* k)
{
	void *val =  dictFetchValue(m_internel->m_dict,k);
	return NULL != val ? static_cast<V*>(val) : NULL;
}

template<typename K,typename V>
int CHashMap<K,V>::put(K* k,V* v)
{
	int ret = dictReplace(m_internel->m_dict,k,v);
	//ret 0 if update,else ret 1 if new assoc
	return ret;
}

template<typename K,typename V>
bool CHashMap<K,V>::insert(K* k,V* v)
{
	int ret = dictAdd(m_internel->m_dict,k,v);
	return ret == 0 ? true : false;
}

template<typename K,typename V>
__Iterator<K,V> CHashMap<K,V>::getIterator()
{
	dictIterator* dict_iterator = dictGetSafeIterator(m_internel->m_dict);
	__Iterator<K,V> iterator;
	iterator.d->iterator = dict_iterator;
	return iterator;
}

template<typename K,typename V>
bool CHashMap<K,V>::next(Iterator& iterator)
{
	dictEntry * entry = dictNext((dictIterator*)iterator.d->iterator);
	iterator.d->entry = entry;
	return NULL != entry ? true : false;
}

template<typename K,typename V>
uint32_t CHashMap<K,V>::size()
{
	unsigned long size =  dictSize(m_internel->m_dict);
	assert(size <= 0xFFFFFFFF);
	return static_cast<uint32_t>(size);
}

template<typename K,typename V>
bool CHashMap<K,V>::empty()
{
	return NULL == m_internel->m_dict || 0 == size() ? true : false;
}

template<typename K,typename V>
void CHashMap<K,V>::clear()
{
	dictEmpty(m_internel->m_dict);
}

template<typename K,typename V>
bool CHashMap<K,V>::erase(const K* k,bool needFree)
{
	int ret = 0;
	if(needFree){
		ret = dictDelete(m_internel->m_dict,k);
	}else{
		ret = dictDeleteNoFree(m_internel->m_dict,k);
	}

	return ret == DICT_OK ? true : false;
}

template<typename K,typename V>
void CHashMap<K,V>::swap(CHashMap<K,V>& rhs)
{
	std::swap(m_internel,rhs.m_internel);	
}

} //namespace Dahua
} //namespace EFS

#endif //__INCLUDE_DAHUA_CLOUD_EFS_HASH_MAP_H__
#endif
