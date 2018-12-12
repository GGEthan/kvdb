#ifndef _ENGINE_MEMINDEX_H_
#define _ENGINE_MEMINDEX_H_

#include "BaseType.h"

#include <mutex>

#include <map>

namespace kv_engine {

// ThreadSafe Index
class MemIndex {
public:
    MemIndex(){}
    virtual ~MemIndex(){}

    virtual Status Put(const KeyType & key, const ValueType & value, const bool overwrite) = 0;

    virtual Status Get(const KeyType & key, ValueType & value) = 0;

    virtual Status Scan(const KeyType & start, const int record_count, ScanHandle & handle) = 0;
    
    virtual Status Delete(const KeyType & key) = 0;

	virtual size_t size() = 0;
};

// Use std::map as Index
class RBTree : public MemIndex {
public:
    RBTree(){}
    virtual ~RBTree(){}
	virtual Status Put(const KeyType & key, const ValueType & value, const bool overwrite) override;
	
    virtual Status Get(const KeyType & key, ValueType & value) override;

    virtual Status Scan(const KeyType & start, const int record_count, ScanHandle & handle) override;
    
    virtual Status Delete(const KeyType & key) override;

    virtual size_t size() override;
private:
    std::map<KeyType, ValueType> _map;
	std::mutex _mtx;
};

} // namespace kv_engine

#endif // _ENGINE_MEMINDEX_H_