#ifndef _ENGINE_MEMTABLE_H_
#define _ENGINE_MEMTABLE_H_

#include "BaseType.h"

namespace kv_engine {

class MemTable {
public:
    MemTable() { }
    Status Put(const KeyType & key, const ValueType & value, const bool overwrite);

    Status Get(const KeyType & key, ValueType & value);

    Status Scan(const KeyType & start, const int record_count, ScanHandle & handle);
    
    Status Delete(const KeyType & key);
private:
    
};

}


#endif //_ENGINE_MEMTABLE_H_