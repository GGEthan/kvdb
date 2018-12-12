#ifndef _ENGINE_MEMINDEX_H_
#define _ENGINE_MEMINDEX_H_

#include "BaseType.h"

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
};

// Use std::map as Index
class RBTree : public MemIndex {
public:
    RBTree(){}
    virtual ~RBTree(){}

private:
    std::map<KeyType, ValueType> _map;

};

} // namespace kv_engine

#endif // _ENGINE_MEMINDEX_H_