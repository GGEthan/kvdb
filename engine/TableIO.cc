
#include "TableIO.h"

#include "Configuration.h"

#include "Meta.h"

#include <fcntl.h>

#include <unistd.h>

namespace kv_engine {

void TableWriter::WriteTableBackgroud(MemTable * mem) {
    // The begin of the writer thread

    unique_lock<mutex> ulock(mem->_writers_mtx);
    while (mem->_writers != 0) {
        // TODO: add max wait time
        mem->_writers_cv.wait(ulock);
    }
    if (mem->ApproximateMemorySize() == 0) {
        delete mem;
        return;
    }
    TableWriter writer(mem);
    
    writer.Init();
    
    writer.WriteTable();

    Meta * meta = Configuration::meta;
    meta->NewSSTable(mem->id, 0);

    delete mem;

    TryToCompactSSTable();
}

void TableWriter::TryToCompactSSTable() {
    static mutex mtx;
    lock_guard<mutex> guard(mtx);
    Meta * meta = Configuration::meta;
    int level = 0;
    while(true) {
        if(meta->size(level) > Configuration::COMPACT_SIZE) {
            long id;
            Status res = CompactSSTable(level, id);
            if (res != Success) {
                ERRORLOG("Compact SSTable Fail!!!![%d]", level);
                return;
            }
            Configuration::TableReaderMap[id] = new TableReader(level + 1, id);
            meta->Compact(level, id);
            level ++;
        }
        else
            break;
    }
}

Status TableWriter::CompactSSTable(int level, long & new_id) {
    TableReader * readers[32];
    TableReader::Iterator iters[32];
    Meta * meta = Configuration::meta;

    auto iter = meta->Level(level);
    for (int i = 0; i < Configuration::COMPACT_SIZE; i++) {
        auto table_info = iter.next();
        if (table_info == nullptr)
            return FileNotFound;
        auto reader = Configuration::TableReaderMap.find(table_info->id);
        if (reader == Configuration::TableReaderMap.end())
            return FileNotFound;
        if (reader->second == nullptr)
            return FileNotFound;
        readers[i] = reader->second;
        iters[i] = reader->second->Begin();
        iters[i].next(); // prepare the first entry
    }

    long id = Configuration::meta->new_id();
    TableWriter writer;
    if (writer.Init(level + 1, id) != Success)
        return FileNotFound;

    KeyType lastKey;
    size_t offset = 0;
    writer._index.resize(0);
    while(true) {
        // compact
        KeyType min;
        
        int min_index = -1;

        for (int i = 0; i < Configuration::COMPACT_SIZE; i++) {
            if (!iters[i].end()) {
                if (min_index == -1) {
                    min_index = i;
                    iters[i].ReadKey(min);
                }
                else {
                    KeyType temp;
                    iters[i].ReadKey(temp);
                    // Cannot use <=
                    if (temp < min) {
                        min_index = i;
                        min.replace(temp);
                    }
                }
            }
        }
        if (min_index == -1)
            break;
        else {
            if (min == lastKey) {
                iters[min_index].next();
                continue;
            }
            lastKey.copy(min);
            ValueType min_v;
            iters[min_index].ReadRecord(min, min_v);
            writer.WriteRecord(min, min_v);
            iters[min_index].next();
            
            writer._index.push_back(offset);
            offset += min.size() + min_v.size() + 2 * sizeof(unsigned int);
        }
    }

    // write index
    
    size_t index_size = sizeof(size_t) * writer._index.size();
    size_t res = ::write(writer._fd, writer._index.data(), index_size);
    if (res != index_size)
        return IOError;
    // write foot
    res = ::write(writer._fd, &index_size, sizeof(size_t));
    if (res != sizeof(size_t))
        return IOError;
    
    new_id = id;

    return Success;
}

TableWriter::TableWriter() {

}

TableWriter::TableWriter(MemTable * mem) {
    _mem = mem;
}

TableWriter::~TableWriter() {
    if (_fd != -1)
        ::close(_fd);
}

Status TableWriter::Init() {
    if (_mem == nullptr) {
        ERRORLOG("MemTable not set.[TableWriter::Init()]");
        return FileNotFound;
    }
    return Init(_mem);
}

Status TableWriter::Init(MemTable * mem) {
    _mem = mem;
    return Init(0, _mem->id);
}

Status TableWriter::Init(int level, long id) {
    std::string file_name = ConcatFileName(Configuration::DATA_DIR, Configuration::SSTABLE_NAME, level, id);
    INFOLOG("Writing %s", file_name.data());
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
    size_t total_size = 0;
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
    if ((value_size & 1<31) != 0) {
        // removed
        key.assign(key_buf, key_size);
        value.removed = true;
    }
    else {
        value_buf = new char[value_size];
        res = pread(_fd, value_buf, value_size, offset + sizeof(unsigned int) * 2 + key_size);
        if (res != value_size) {
            delete key_buf;
            delete value_buf;
            return IOError;
        }
        key.assign(key_buf, key_size);
        value.assign(value_buf, value_size);
    }

    return Success;
}

Status TableReader::Find(const KeyType & key, ValueType & value) {
    return _BinarySearch(key, value);
}

TableReader::Iterator::Iterator(TableReader * reader) {
    _reader = reader;
}

TableReader::Iterator::Iterator(const TableReader::Iterator & that) {
    _reader = that._reader;
    _offset = that._offset;
}

void TableReader::Iterator::FromReader(TableReader * reader) {
    _reader = reader;
} 

bool TableReader::Iterator::next() {
    if (_offset >= _reader->_index_offset)
        return false;

    if (_reader->_ReadRecord(_offset, _key, _value) != Success)
        return false;

    if (_value.removed)
        _offset += 2 * (sizeof(unsigned int)) + _key.size();
    else
        _offset += 2 * (sizeof(unsigned int)) + _key.size() + _value.size();

    return true;
}

bool TableReader::Iterator::end() {
    return _offset >= _reader->_index_offset;
}

void TableReader::Iterator::ReadRecord(KeyType & key, ValueType & value) {
    key.replace(_key);
    value.replace(_value);
}

void TableReader::Iterator::ReadKey(KeyType & key) {
    key.replace(_key);
}

Status TableReader::_ReadIndex(int n, size_t & index) {
    off_t offset = _index_offset + sizeof(size_t) * n;
    if (_index_offset >= _size - sizeof(size_t))
        return IOError;
    if (::pread(_fd, &index, sizeof(size_t), offset) != sizeof(size_t))
        return IOError;
    return Success;
}

Status TableReader::_ReadKey(size_t offset, KeyType & key) {
    if (offset >= _index_offset)
        return IOError;
    unsigned int key_size;
    char* buf;
    size_t res = ::pread(_fd, &key_size, sizeof(unsigned int), offset);
    if (res != sizeof(unsigned int))
        return IOError;
    buf = new char[key_size];
    key.assign(buf, key_size);

    res = ::pread(_fd, buf, key_size, offset + sizeof(unsigned int));
    if (res != key_size)
        return IOError;
    return Success;
}

Status TableReader::_ReadValue(size_t offset, ValueType & value) {
    if (offset >= _index_offset)
        return IOError;
    unsigned int value_size;
    char* buf;
    size_t res = ::pread(_fd, &value_size, sizeof(unsigned int), offset);
    if (res != sizeof(unsigned int))
        return IOError;
    if ((value_size & (1<<31)) != 0) {
        //removed
        value.removed = true;
    }
    else {
        buf = new char[value_size];
        value.assign(buf, value_size);
        res = ::pread(_fd, buf, value_size, offset + sizeof(unsigned int));
        if (res != value_size)
            return IOError;
    }
    return Success;
}

Status TableReader::_ReadKeyWithIndex(int index, KeyType & key, size_t & offset) {
    if (_ReadIndex(index, offset) != Success)
        return IOError;
    if (_ReadKey(offset, key) != Success)
        return IOError;
    return Success;
}

Status TableReader::_BinarySearch(const KeyType & key, ValueType & value) {
    KeyType mid_key;
    size_t offset;
    int index_count = (_size - _index_offset - sizeof(size_t)) / sizeof(size_t);
    int left = 0, right = index_count - 1;
    while (left <= right) {
        int mid = left + ((right - left) >> 1);
        if (_ReadKeyWithIndex(mid, mid_key, offset) != Success)
            return IOError;
        int cmp = mid_key.compare(key);
        if (cmp > 0) // mid > k
            right = mid - 1;
        else if (cmp < 0) // mid < k
            left = mid + 1;
        else {
            if (_ReadValue(offset + sizeof(unsigned int) + mid_key.size(), value) != Success)
                return IOError;
            return Success;
        }      
    }
    return KeyNotFound;
}

} // namespace kv_engine