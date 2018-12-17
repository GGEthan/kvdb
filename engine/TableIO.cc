
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

    std::string file_name = ConcatFileName(Configuration::DATA_DIR, Configuration::SSTABLE_NAME, 0, _mem->id);
    
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
    delete iter; // TODO : smart pointer
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

TableReader::TableReader(int level, long id) {
    _level = level;
    _id = id;
}

TableReader::TableReader() {

}

Status TableReader::Init(int level, long id) {
    _level = level;
    _id = id;
    return Init();
}

Status TableReader::Init() {
    if (_level < 0 || _id < 0) {
        ERRORLOG("Wrong level:%d or id:%ld.[TableReader::Init()]", _level, _id);
        return FileNotFound;
    }
    std::string file_name = ConcatFileName(Configuration::DATA_DIR, Configuration::SSTABLE_NAME, _level, _id);
    int fd = ::open(file_name.data(), O_RDONLY);
    if (fd <= 0) {
        ERRORLOG("Can't Open SSTable %s.", file_name.data());
        return FileNotFound;
    }
    _fd = fd;

    off_t index_size_offset = lseek(_fd, -sizeof(size_t), SEEK_END);
    
    _size = index_size_offset + sizeof(size_t);

    size_t index_size = 0;
    if (pread(_fd, &index_size, sizeof(size_t), index_size_offset) != sizeof(size_t)) {
        ERRORLOG("Invalid SSTable File.");
        return IOError;
    }

    _index_offset = index_size_offset - index_size;

    return Success;
}

TableReader::Iterator TableReader::Begin() {
    return TableReader::Iterator(this);
}

Status TableReader::_ReadRecord(size_t offset, KeyType & key, ValueType & value) {
    unsigned int key_size = 0;
    unsigned int value_size = 0;
    char * key_buf = nullptr;
    char * value_buf = nullptr;
    size_t res = pread(_fd, &key_size, sizeof(unsigned int), offset);
    if (res != sizeof(unsigned int))
        return IOError;

    key_buf = new char[key_size];
    res = pread(_fd, key_buf, key_size, offset + sizeof(unsigned int));
    if (res != key_size) {
        delete key_buf;
        return IOError;
    }

    res = pread(_fd, &value_size, sizeof(unsigned int), offset + sizeof(unsigned int) + key_size);
    if (res != sizeof(unsigned int)) {
        delete key_buf;
        return IOError;
    }
    if (value_size & 1<31 != 0) {
        // removed
        key.assign(key_buf, key_size);
        value.removed = true;
    }
    else {
        value_buf = new char[value_size];
        res = pread(_fd, value_buf, value_size, offset + sizeof(unsigned int) * 2 + key_size);
        if (res != sizeof(value_size)) {
            delete key_buf;
            delete value_buf;
            return IOError;
        }
        key.assign(key_buf, key_size);
        value.assign(value_buf, value_size);
    }

    return Success;
}

TableReader::Iterator::Iterator(TableReader * reader) {
    _reader = reader;
}

TableReader::Iterator::Iterator(const TableReader::Iterator & that) {
    _reader = that._reader;
    _offset = that._offset;
}

bool TableReader::Iterator::next() {
    if (_offset >= _reader->_index_offset)
        return false;

    if (_reader->_ReadRecord(_offset, _key, _value) != Success)
        return false;

    _offset += 2 * (sizeof(unsigned int)) + _key.size() + _value.size();

    return true;
}

void TableReader::Iterator::ReadRecord(KeyType & key, ValueType & value) {
    key.replace(_key);
    value.replace(_value);
}

void TableReader::Iterator::ReadKey(KeyType & key) {
    key.replace(_key);
}

} // namespace kv_engine