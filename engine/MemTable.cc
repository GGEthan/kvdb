#include "MemTable.h"

#include "Configuration.h"

#include "Util.h"

namespace kv_engine {


MemTable* MemTable::memTable = nullptr;
MemTable* MemTable::immutableTable = nullptr;
void MemTable::SetImmutable() {
	static mutex mtx;
	lock_guard<mutex> guard(mtx);
	if (!_mutable)
		return;
	unique_lock<mutex> ulock(change_mtx);
	while (immutableTable != nullptr)
		change_cv.wait(ulock);
	
	// set new MemTable and make this to immutableTable
	memTable = new MemTable();
	immutableTable = this;
	// TODO : persist immutableTable

	_mutable = false;
}

Status MemTable::Put(const KeyType & key, const ValueType & value, const bool overwrite) {
	
	{
		int x = 0;
		if (_mutable)
	
	}
	int y = x;
	ASSERT(_index->Put(key, value, overwrite));
	std::lock_guard<std::mutex> guard(_size_mtx);
	_size += key.size() + value.size();
	if (_size > Configuration::MAX_MEMTABLE_SIZE && _mutable) {
		
	}
	return Success;
}

Status MemTable::Get(const KeyType & key, const ValueType & value) {

}
} // namespace kv_engine