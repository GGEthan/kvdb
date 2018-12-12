#include "KVEngine.h"

namespace kv_engine {

Status KVEngine::Open(std::string conf_path) {
    return UnknownError;
}


Status KVEngine::Put(const KeyType & key, const ValueType & value, const bool overwrite) {
    return UnknownError;
}

Status KVEngine::Get(const KeyType & key, ValueType & value) {
    return UnknownError;
}

Status KVEngine::Scan(const KeyType & start, const int record_count, ScanHandle & handle) {
    return UnknownError;
}

Status KVEngine::Delete(const KeyType & key) {
    return UnknownError;
}

} // namespace kv_engine