//
//  "$Id$"
//
//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//

#ifndef WIN32
#ifndef __INCLUDE_DAHUA_CLOUD_EFS_LINKED_HASH_MAP_H__
#define __INCLUDE_DAHUA_CLOUD_EFS_LINKED_HASH_MAP_H__

#include "HashMap.h"
#include "Infra/IntTypes.h"
#include "Infra/Mutex.h"
#include "Infra/Guard.h"

namespace Dahua{
namespace EFS{

#define LINK_MAP_SIZE 256

template<typename K,typename V>
class CLinkedHashMap {
public:
	typedef struct __Iterator<K,V> Iterator;
    explicit CLinkedHashMap(
        void* pridata,HashMapFuncs& funcs,
        HashMapFuncs::HashFunc h = NULL,
        bool locked=false
    );

    ~CLinkedHashMap();

    //获取值,不存在则返回NULL
    //初始化locked=true则外部通过lock(k)加锁保证线程安全
    V* get(const K* k);
    
    //键不存在,则插入,返回 1;键存在则覆盖记录,返回0
    //初始化locked=true则外部通过lock(k)加锁保证线程安全
    int put(K* k,V* v);

    //键不存在,则插入,返回 true;键存在则失败,返回false
   //初始化locked=true则外部通过lock(k)加锁保证线程安全
    bool insert(K* k,V* v);

    //获取迭代器,总是指向begin
    //该接口不提供lock版本  
    Iterator getIterator();

    //还有值，返回true,否则返回false
    //该接口不提供lock版本
    bool next(Iterator & iterator);

    //获取map的长度，locked=true线程安全
    uint32_t size();
    
    //map是否没有元素，空返回true,否则返回false
    //locked=true线程安全
    bool empty();
    
    //清空map,locked=true线程安全
    void clear();
    
    //删除一个k所对应的kv对，value可以选择是否要释放
    //初始化locked=true则外部通过lock(k)加锁保证线程安全
    bool erase(const K* k,bool needFree=true);
    
    //两个linkedhashmap交换内部数据
    //locked=true线程安全
    void swap(CLinkedHashMap<K,V>& rhs);

    //初始化locked=true,显式的拿到某个subbucket的锁
    void lock(uint32_t id);
    //初始化locked=true,显式的释放某个subbucket的锁
    void unlock(uint32_t id);

    //初始化locked=true,对某个key进行加锁
    void lock(const K* key);
    //初始化locked=true,对某个key进行解锁
    void unlock(const K* key);

    //返回linkedhashmap中包含的subbucket个数
    uint32_t getSubBucketNum();
    //获取某个subbuket的迭代器
    Iterator getSubBucketIterator(uint32_t id);
    //获取某个key所属于那个subbucket号
    uint32_t getSubBucketSlot(const K* k);
    //迭代某个subbucket
    bool nextSubBucket(uint32_t id,Iterator & iterator);
private:
    CLinkedHashMap();
    CLinkedHashMap(const CLinkedHashMap& rhs);
    CLinkedHashMap& operator=(const CLinkedHashMap& rhs);   
private:
    bool m_need_locked;
    uint32_t m_iterator_index;
    HashMapFuncs::HashFunc m_hash_func;
    CHashMap<K,V>* m_linked_map[LINK_MAP_SIZE]; //256 hashmap
    Infra::CMutex m_slot_mutex[LINK_MAP_SIZE];
};

template<typename K,typename V>
CLinkedHashMap<K,V>::CLinkedHashMap(
    void* pridata,HashMapFuncs& funcs,
    HashMapFuncs::HashFunc h,bool locked)
{
    m_iterator_index = 0;
    m_hash_func = h == NULL ? funcs.hash : h;
    for(int i = 0; i < LINK_MAP_SIZE; ++i){
        m_linked_map[i] = new CHashMap<K,V>(pridata,funcs);
    }
    m_need_locked = locked;
}

template<typename K,typename V>
CLinkedHashMap<K,V>::~CLinkedHashMap()
{
    m_iterator_index = 0;
    for(int i = 0; i < LINK_MAP_SIZE; ++i){
        delete m_linked_map[i];
        m_linked_map[i] = NULL;
    }
}

template<typename K,typename V>
V* CLinkedHashMap<K,V>::get(const K* k)
{
    uint32_t link_index = m_hash_func(k) % LINK_MAP_SIZE;
    return m_linked_map[link_index]->get(k);
}

template<typename K,typename V>
int CLinkedHashMap<K,V>::put(K* k,V* v)
{
    uint32_t link_index = m_hash_func(k) % LINK_MAP_SIZE;
    return m_linked_map[link_index]->put(k,v);
}

template<typename K,typename V>
bool CLinkedHashMap<K,V>::insert(K* k,V* v)
{
    uint32_t link_index = m_hash_func(k) % LINK_MAP_SIZE;
    return m_linked_map[link_index]->insert(k,v);
}

template<typename K,typename V>
__Iterator<K,V> CLinkedHashMap<K,V>::getIterator()
{
    Iterator linked_iterator = m_linked_map[0]->getIterator();
    m_iterator_index = 0;
	return linked_iterator;
}

template<typename K,typename V>
bool CLinkedHashMap<K,V>::next(Iterator & iterator)
{
    bool ret = m_linked_map[m_iterator_index]->next(iterator);
    if(!ret && m_iterator_index < LINK_MAP_SIZE){
        //循环判断，直到找到一个可用的为止。
        for(; ++m_iterator_index < LINK_MAP_SIZE; ){
            Iterator linked_iterator = m_linked_map[m_iterator_index]->getIterator();
            ret = m_linked_map[m_iterator_index]->next(linked_iterator);
            if(ret){
				iterator = linked_iterator;
                break;
            }
        }
    }

    return ret;
}

template<typename K,typename V>
uint32_t CLinkedHashMap<K,V>::size()
{
    uint32_t size = 0;
    for(int i = 0; i < LINK_MAP_SIZE; ++i){
        if( m_need_locked ){
            Infra::CGuard lock(m_slot_mutex[i]);
            size += m_linked_map[i]->size();
        }else{
            size += m_linked_map[i]->size();
        }
    }

    return size;
}

template<typename K,typename V>
bool CLinkedHashMap<K,V>::empty()
{
    return size() == 0;
}

template<typename K,typename V>
void CLinkedHashMap<K,V>::clear()
{
    for(int i = 0; i < LINK_MAP_SIZE; ++i){
        if( m_need_locked ){
            Infra::CGuard lock(m_slot_mutex[i]);
            m_linked_map[i]->clear();
        }else{
            m_linked_map[i]->clear();
        }
    }   
}

template<typename K,typename V>
bool CLinkedHashMap<K,V>::erase(const K* k,bool needFree)
{
    uint32_t link_index = m_hash_func(k) % LINK_MAP_SIZE;
    return m_linked_map[link_index]->erase(k,needFree);
}

template<typename K,typename V>
void CLinkedHashMap<K,V>::swap(CLinkedHashMap<K,V>& rhs)
{
    for(int i = 0; i < LINK_MAP_SIZE; ++i){
        if( m_need_locked ){
            Infra::CGuard lock(m_slot_mutex[i]);
            m_linked_map[i]->swap(*rhs.m_linked_map[i]);
        }else{
            m_linked_map[i]->swap(*rhs.m_linked_map[i]);
        }
    }

    if( m_need_locked ){
        Infra::CGuard lock(m_slot_mutex[0]);
    	std::swap(m_iterator_index,rhs.m_iterator_index);
    	std::swap(m_hash_func,rhs.m_hash_func);
    }else{
        std::swap(m_iterator_index,rhs.m_iterator_index);
        std::swap(m_hash_func,rhs.m_hash_func);
    }
}

//初始化locked=true,显式的拿到某个subbucket的锁
template<typename K,typename V>
void CLinkedHashMap<K,V>::lock(uint32_t id)
{
    m_slot_mutex[id].enter();
}

//初始化locked=true,显式的释放某个subbucket的锁
template<typename K,typename V>
void CLinkedHashMap<K,V>::unlock(uint32_t id)
{
    m_slot_mutex[id].leave();
}


//初始化locked=true,对某个key进行加锁
template<typename K,typename V>
void CLinkedHashMap<K,V>::lock(const K* key)
{
    uint32_t link_index = m_hash_func(key) % LINK_MAP_SIZE;
    m_slot_mutex[link_index].enter();
}
//初始化locked=true,对某个key进行解锁
template<typename K,typename V>
void CLinkedHashMap<K,V>::unlock(const K* key)
{
    uint32_t link_index = m_hash_func(key) % LINK_MAP_SIZE;
    m_slot_mutex[link_index].leave();
}

//返回linkedhashmap中包含的subbucket个数
template<typename K,typename V>
uint32_t CLinkedHashMap<K,V>::getSubBucketNum()
{
    return LINK_MAP_SIZE;
}

//获取某个subbuket的迭代器
template<typename K,typename V>
__Iterator<K,V> CLinkedHashMap<K,V>::getSubBucketIterator(uint32_t id)
{
    Iterator linked_iterator = m_linked_map[id]->getIterator(); 
    return linked_iterator;
}

//获取某个key所属于那个subbucket号
template<typename K,typename V>
uint32_t CLinkedHashMap<K,V>::getSubBucketSlot(const K* k)
{
    uint32_t link_index = m_hash_func(k) % LINK_MAP_SIZE;
    return link_index;
}

//迭代某个subbucket
template<typename K,typename V>
bool CLinkedHashMap<K,V>::nextSubBucket(uint32_t id,Iterator & iterator)
{
    bool ret = m_linked_map[id]->next(iterator);
    return ret;
}

} //namespace EFS
} //namespace Dahua

#endif //__INCLUDE_DAHUA_CLOUD_EFS_LINKED_HASH_MAP_H__
#endif
