
#include "BaseType.h"

#include "MemIndex.h"

#include <map>

#include <mutex>

namespace kv_engine {
using std::pair;
Status RBTree::Put(const KeyType & key, const ValueType & value, const bool overwrite) {
	std::lock_guard<std::mutex> guard(_mtx);
	if (overwrite)
		_map[key] = value;
	else {
		if (_map.find(key) != _map.end())
			return KeyExists;
		_map.insert(pair<KeyType, ValueType>(key, value));
	}
	return Success;
}

Status RBTree::Get(const KeyType & key, ValueType & value) {
	std::lock_guard<std::mutex> guard(_mtx);
	auto iter = _map.find(key);
	if (iter == _map.end())
		return KeyNotFound;
	if (value.removed)
		return KeyRemove;
	value = iter->second;
	return Success;
}

Status RBTree::Scan(const KeyType & start, const int record_count, ScanHandle & handle) {
	// TODO : scan
	return UnknownError;
}

Status RBTree::Delete(const KeyType & key) {
	std::lock_guard<std::mutex> guard(_mtx);
	ValueType removed;
	removed.removed = true;
	_map[key] = removed;
	return Success;
}

size_t RBTree::size() {
	return _map.size();
}
} // namespace kv_engine