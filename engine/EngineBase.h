#ifndef _ENGINE_ENGINE_BASE_H_
#define _ENGINE_ENGINE_BASE_H_

#include "BaseType.h"

namespace kv_engine {

class EngineBase {
 
public:
    EngineBase(){}

    virtual ~EngineBase(){}

    virtual Status Open(std::string config_path) = 0;

    virtual Status Put(const KeyType & key, const ValueType & value, const bool overwrite) = 0;

    virtual Status Get(const KeyType & key, ValueType & value) = 0;

    virtual Status Scan(const KeyType & start, const int record_count, ScanHandle & handle) = 0;
    
    virtual Status Delete(const KeyType & key) = 0;

};

} // namespace kv_engine

#endif // _ENGINE_ENGINE_BASE_H_