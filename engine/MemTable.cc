#include "MemTable.h"

#include "Configuration.h"

#include "Util.h"

#include "TableIO.h"

#include <thread>

namespace kv_engine {


MemTable* MemTable::memTable = nullptr;
MemTable* MemTable::immutableTable = nullptr;
mutex MemTable::change_mtx;
condition_variable MemTable::change_cv;

MemTable::MemTable() {
	_index = new RBTree();
	//id = time(NULL);
	id = Configuration::meta->new_id();
	_log = new DBLog();
	_log->Open(id);
}

MemTable::~MemTable() {
	if (immutableTable == this)
		immutableTable = nullptr;
	change_mtx.unlock();
	// TODO : release resource
	unique_lock<mutex> ulock(_readers_mtx);
	while(_readers != 0)
		_readers_cv.wait(ulock);
	
	delete _index;
	
	_index = nullptr;

	delete _log;
}

// bool MemTable::testTableSize() {
// 	lock_guard<mutex> guard(_size_mtx);
// 	if (!_mutable)
// 		return false;
// 	if (_size > Configuration::MAX_MEMTABLE_SIZE){
// 		if (_mutable) 
// 			setImmutable();
// 		return false;
// 	}
// 	return true;
// }

Status MemTable::SafeSetImmutable() {
	static mutex mtx;
	lock_guard<mutex> guard(mtx);
	return SetImmutable();
}

Status MemTable::SetImmutable() {
	if (!_mutable) 
		return Success;
	
	change_mtx.lock(); // try to lock, wait until release the old one
	immutableTable = this;

	std::thread write_thread(TableWriter::WriteTableBackgroud, this);
	write_thread.detach();

	memTable = new MemTable();

	_mutable = false;
	return Success;
}

size_t MemTable::ApproximateMemorySize() {
	// TODO : Cannot be such easy here.
	return _size;
	
}

Status MemTable::Get(const KeyType & key, ValueType & value) {
	readerIn();

	Status res = _index->Get(key, value);

	readerOut();
	return res;
}


Status MemTable::Put(const KeyType & key, const ValueType & value, const bool overwrite) {
	writerIn();

	_log->Log(key, value);

	Status res = _index->Put(key, value, overwrite);

	writerOut();
	if (res != Success)
		return res;
	std::lock_guard<std::mutex> guard(_size_mtx);
	_size += key.size() + value.size();
	return Success;
}

Status MemTable::Delete(const KeyType & key) {
	writerIn();

	ValueType delete_value = ValueType();
	delete_value.removed = true;

	_log->Log(key, delete_value);

	Status res = _index->Delete(key);

	writerOut();
	std::lock_guard<std::mutex> guard(_size_mtx);
	_size += key.size();
	return res;
}

Status MemTable::Scan(const KeyType & start, const int record_count, ScanHandle & handle) {
	// TODO : Scan
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