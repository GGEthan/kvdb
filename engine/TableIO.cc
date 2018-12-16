
#include "TableIO.h"

#include "Configuration.h"

#include <fcntl.h>

#include <unistd.h>

namespace kv_engine {

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

} // namespace kv_engine