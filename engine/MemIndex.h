#ifndef _ENGINE_MEMINDEX_H_
#define _ENGINE_MEMINDEX_H_

#include "BaseType.h"

#include <mutex>

#include <map>

namespace kv_engine {

class ScanHandle {
public:
    ScanHandle(){}
    virtual ~ScanHandle(){}

    virtual Status GetKeyValue(KeyType & key, ValueType & value) = 0;

    virtual bool GetNext() = 0;

};
// ThreadSafe Index
class MemIndex {
public:
    MemIndex(){}
    virtual ~MemIndex(){}


    virtual Status Put(const KeyType & key, const ValueType & value, const bool overwrite) = 0;

    virtual Status Get(const KeyType & key, ValueType & value) = 0;

    virtual Status Scan(const KeyType & start, const int record_count, ScanHandle & handle) = 0;
    
    virtual Status Delete(const KeyType & key) = 0;

    virtual ScanHandle* GetIterator() = 0; // remember free the handle

	virtual size_t size() = 0;
};

// Use std::map as Index
class RBTree : public MemIndex {
public:
    RBTree(){}

	virtual Status Put(const KeyType & key, const ValueType & value, const bool overwrite) override;
	
    virtual Status Get(const KeyType & key, ValueType & value) override;

    virtual Status Scan(const KeyType & start, const int record_count, ScanHandle & handle) override;
    
    virtual Status Delete(const KeyType & key) override;

    virtual ScanHandle* GetIterator()override;

    virtual size_t size() override;
private:
    std::map<KeyType, ValueType> _map;
	std::mutex _mtx;
};

class RBTreeScanHandle : public ScanHandle {
public:
    RBTreeScanHandle(std::map<KeyType, ValueType>* map);

    RBTreeScanHandle(std::map<KeyType, ValueType>* map, const KeyType & key);

    virtual ~RBTreeScanHandle();

    virtual Status GetKeyValue(KeyType & key, ValueType & value) override;

    virtual bool GetNext() override;
private:
    std::map<KeyType, ValueType> * _map = nullptr;

    std::map<KeyType, ValueType>::iterator _iter;

};

} // namespace kv_engine

#endif // _ENGINE_MEMINDEX_H_