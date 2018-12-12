#include "MemTable.h"

#include "Configuration.h"

#include "Util.h"

namespace kv_engine {


MemTable* MemTable::memTable = nullptr;
MemTable* MemTable::immutableTable = nullptr;
mutex MemTable::change_mtx;
condition_variable MemTable::change_cv;

bool MemTable::testTableSize() {
	lock_guard<mutex> guard(_size_mtx);
	if (!_mutable)
		return false;
	if (_size > Configuration::MAX_MEMTABLE_SIZE){
		if (_mutable) 
			setImmutable();
		return false;
	}
	return true;
}

void MemTable::setImmutable() {
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

Status MemTable::Get(const KeyType & key, ValueType & value) {
	readerIn();

	Status res = _index->Get(key, value);

	readerOut();
	return res;
}


Status MemTable::Put(const KeyType & key, const ValueType & value, const bool overwrite) {
	if (!testTableSize())
		return TableFull;

	writerIn();

	Status res = _index->Put(key, value, overwrite);

	writerOut();
	if (res != Success)
		return res;
	std::lock_guard<std::mutex> guard(_size_mtx);
	_size += key.size() + value.size();
	return Success;
}

Status MemTable::Delete(const KeyType & key) {
	if (!testTableSize())
		return TableFull;
	
	writerIn();

	Status res = _index->Delete(key);

	writerOut();

	return res;
}

Status MemTable::Scan(const KeyType & start, const int record_count, ScanHandle & handle) {
	return UnknownError;
}

void MemTable::writerIn() {
	_writers_mtx.lock();
	_writers ++;
	_writers_mtx.unlock();
}

void MemTable::writerOut() {
	_writers_mtx.lock();
	_writers --;
	if (_writers == 0)	
		_writers_cv.notify_all();
	_writers_mtx.unlock();
}

void MemTable::readerIn() {
	_readers_mtx.lock();
	_readers ++;
	_readers_mtx.unlock();
}

void MemTable::readerOut() {
	_readers_mtx.lock();
	_readers --;
	if (_readers == 0)
		_readers_cv.notify_all();
	_readers_mtx.unlock();
}

} // namespace kv_engine