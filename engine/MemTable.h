#ifndef _ENGINE_MEMTABLE_H_
#define _ENGINE_MEMTABLE_H_

#include "BaseType.h"

#include "MemIndex.h"

#include <mutex>

#include <condition_variable>

namespace kv_engine {
using std::mutex;
using std::lock_guard;
using std::condition_variable;
using std::unique_lock;
class MemTable {
public:
	static MemTable* memTable;
	static MemTable* immutableTable;

    MemTable() { 
		_index = new RBTree();
	}
    Status Put(const KeyType & key, const ValueType & value, const bool overwrite);

    Status Get(const KeyType & key, ValueType & value);

    Status Scan(const KeyType & start, const int record_count, ScanHandle & handle);
    
    Status Delete(const KeyType & key);
private:
	static mutex change_mtx;
	static condition_variable change_cv;
	
	MemIndex * _index = nullptr;
	
	mutex _size_mtx;
	size_t _size = 0;
	
	bool _mutable = true;

	int _readers = 0;
	mutex _readers_mtx;
	condition_variable _readers_cv;

	int _writers = 0;
	mutex _writers_mtx;
	condition_variable _writers_cv;

	void setImmutable();
	void addImmutableTable();

	bool testTableSize();

	void writerIn();
	void writerOut();
	void readerIn();
	void readerOut();
};

}


#endif //_ENGINE_MEMTABLE_H_