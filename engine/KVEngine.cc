#include "KVEngine.h"

#include "MemTable.h"

#include "Configuration.h"

namespace kv_engine {

Status KVEngine::Open(std::string conf_path) {
    return UnknownError;
}


Status KVEngine::Put(const KeyType & key, const ValueType & value, const bool overwrite) {
    // Test if the memtable is writable
    if (MemTable::memTable->ApproximateMemorySize() >= Configuration::MAX_MEMTABLE_SIZE)
        MemTable::memTable->SafeSetImmutable();
    
    ASSERT(MemTable::memTable->Put(key, value, overwrite));
    
    return Success;
}

Status KVEngine::Get(const KeyType & key, ValueType & value) {
    return UnknownError;
}

Status KVEngine::Scan(const KeyType & start, const int record_count, ScanHandle & handle) {
    return UnknownError;
}

Status KVEngine::Delete(const KeyType & key) {
    // Test if the memtable is writable
    if (MemTable::memTable->ApproximateMemorySize() >= Configuration::MAX_MEMTABLE_SIZE)
        MemTable::memTable->SafeSetImmutable();
    
    ASSERT(MemTable::memTable->Delete(key));

    return Success;
}

} // namespace kv_engine