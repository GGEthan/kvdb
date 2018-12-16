
#include "TableIO.h"

#include "Configuration.h"

#include "Meta.h"

#include <fcntl.h>

#include <unistd.h>

namespace kv_engine {

void TableWriter::WriteTableBackgroud(MemTable * mem) {
    // The begin of the writer thread

    TableWriter writer(mem);
    
    writer.Init();

    unique_lock<mutex> ulock(mem->_writers_mtx);
    while (mem->_writers != 0) {
        // TODO: add max wait time
        mem->_writers_cv.wait(ulock);
    }
    
    writer.WriteTable();

    Meta * meta = Configuration::meta;
    meta->NewSSTable(mem->id, 0);

    delete mem;

    int level = 0;
    while(true) {
        if(meta->size(level) > Configuration::COMPACT_SIZE) {
            // TODO : Compact table here

            meta->Compact(0, 0);
            level ++;
        }
        else
            break;
    }
    
}

TableWriter::TableWriter(MemTable * mem) {
    _mem = mem;
}

TableWriter::~TableWriter() {
    if (_fd != -1)
        ::close(_fd);
}

Status TableWriter::Init(MemTable * mem) {
    _mem = mem;
    return Init();
}

Status TableWriter::Init() {
    if (_mem == nullptr) {
        ERRORLOG("Memtable not set in TableWriter");
        return FileNotFound;
    }

    std::string file_name = ConcatFileName(Configuration::SSTABLE_NAME, 0, _mem->id);
    
    int fd = open(file_name.data(), O_WRONLY | O_CREAT, 0777);
    if (fd <= 0) {
        ERRORLOG("Cannot create file %s.", file_name.data());
        return IOError;
    }
    _fd = fd;

}

Status TableWriter::WriteTable() {
    if (_mem == nullptr || _fd <= 0) {
        ERRORLOG("TableWriter not ready.");
        return UnknownError;
    }

    MemIndex* index = _mem->_index;

    auto iter = index->GetIterator();

    _index.resize(index->size());

    // write key value
    KeyType key;
    ValueType value;
    int count = 0;
    size_t size = 0;
    while (iter->GetKeyValue(key, value) == Success) {
        _index[count] = size;
        size_t _s = WriteRecord(key, value);
        size += _s;
        count++;
        iter->GetNext();
    }

    // write index
    size_t index_size = sizeof(size_t) * _index.size();
    size_t res = ::write(_fd, _index.data(), index_size);
    if (res != index_size)
        return IOError;
    // write foot
    res = ::write(_fd, &index_size, sizeof(size_t));
    if (res != sizeof(size_t))
        return IOError;

    ::close(_fd);
    _fd = -1;
    return Success;
}

size_t TableWriter::WriteRecord(const KeyType & key, const ValueType & value) {
    // [key-size:4 byte][key : key-size byte]
    // [value-size:4 byte][value: value-size byte]
    // value-size & (1<<31) != 0 when deleted
    size_t total_size;
    unsigned int ksize = key.size();
    unsigned int vsize = value.size();

    // key
    total_size += ::write(_fd, &ksize, sizeof(unsigned int));
    total_size += ::write(_fd, key.data(), ksize);

    // value
    if (value.removed) {
        vsize |= 1 << 31;
        total_size += ::write(_fd, &vsize, sizeof(unsigned int));
    }
    else {
        total_size += ::write(_fd, &vsize, sizeof(unsigned int));
        total_size += ::write(_fd, value.data(), vsize);
    }
    return total_size;
}

} // namespace kv_engine