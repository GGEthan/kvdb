#ifndef _ENGINE_KVENGINE_H_
#define _ENGINE_KVENGINE_H_

#include "BaseType.h"

#include "EngineBase.h"

#include "MemIndex.h"

namespace kv_engine {


class KVEngine : public EngineBase {
public:
    KVEngine(){}
    virtual ~KVEngine();
    virtual Status Open(const std::string & conf_path) override ;

    virtual Status Open(const std::string & log_dir, const std::string & data_dir) override;

    virtual Status Put(const KeyType & key, const ValueType & value, const bool overwrite) override;

    virtual Status Get(const KeyType & key, ValueType & value) override;

    virtual Status Scan(const KeyType & start, const int record_count, ScanHandle & handle) override;
    
    virtual Status Delete(const KeyType & key) override;

private:
    
};

} // namespace kv_engine
#endif // _ENGINE_KVENGINE_H_