
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

ScanHandle* RBTree::GetIterator() { 
	return new RBTreeScanHandle(&_map);
}
size_t RBTree::size() {
	return _map.size();
}


RBTreeScanHandle::RBTreeScanHandle(std::map<KeyType, ValueType>* map) {
	_map = map;
	_iter = _map->begin();
}

RBTreeScanHandle::RBTreeScanHandle(std::map<KeyType, ValueType>* map, const KeyType & key) {
	_map = map;
	_iter = _map->find(key);
}


RBTreeScanHandle::~RBTreeScanHandle() {

}

Status RBTreeScanHandle::GetKeyValue(KeyType & key, ValueType & value) {
	if (_iter == _map->end()) {
		return KeyNotFound;
	}
	// key = _iter->first;
	// value = _iter->second;
	key.replace(_iter->first);
	value.replace(_iter->second);
	return Success;
}

bool RBTreeScanHandle::GetNext() {
	_iter++;
	if (_iter == _map->end()) 
		return false;
	return true;
}

} // namespace kv_engine