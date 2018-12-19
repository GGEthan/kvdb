#include "DBLog.h"

#include "Configuration.h"

#include "MemTable.h"

#include <fcntl.h>

#include <unistd.h>

namespace kv_engine {

DBLog::~DBLog() {
    // delete log file here
    ::close(_fd);
    string log_name = ConcatFileName(Configuration::LOG_DIR, Configuration::DBLOG_NAME, 0, _id);
    ::remove(log_name.data());

}

Status DBLog::Log(const KeyType & key, const ValueType & value) {
    // [key-size:4 byte][key : key-size byte]
    // [value-size:4 byte][value: value-size byte]
    // value-size & (1<<31) != 0 when deleted

    size_t cur_size;
    _size_mtx.lock();
    cur_size = _size;
    _size += key.size() + value.size() + 2 * sizeof(unsigned int);
    _size_mtx.unlock();

    size_t total_size = 0;
    unsigned int ksize = key.size();
    unsigned int vsize = value.size();

    // key
    total_size += ::pwrite(_fd, &ksize, sizeof(unsigned int), cur_size + total_size);
    total_size += ::pwrite(_fd, key.data(), ksize, cur_size + total_size);

    // value
    if (value.removed) {
        vsize |= 1 << 31;
        total_size += ::pwrite(_fd, &vsize, sizeof(unsigned int), cur_size + total_size);
    }
    else {
        total_size += ::pwrite(_fd, &vsize, sizeof(unsigned int), cur_size + total_size);
        total_size += ::pwrite(_fd, value.data(), vsize, cur_size + total_size);
    }
    if (total_size == key.size() + value.size() + 2 * sizeof(unsigned int))
        return Success;
    else {
        ERRORLOG("Log Error");
        return IOError;
    }
}

Status DBLog::Open(const long & id) {
    string log_name = ConcatFileName(Configuration::LOG_DIR, Configuration::DBLOG_NAME, 0, id);
    int fd = ::open(log_name.data(), O_WRONLY | O_CREAT | O_APPEND, 0777);
    if (fd <= 0) {
        ERRORLOG("Can't create log file %s.", log_name.data());
        return FileNotFound;
    }    
    _fd = fd;
    _id = id;
    return Success; 
}

Status DBLog::Recover(MemTable * _mem, long id) {
    string log_name = ConcatFileName(Configuration::LOG_DIR, Configuration::DBLOG_NAME, 0, id);
    int fd = ::open(log_name.data(), O_RDONLY);
    if (fd <= 0) {
        ERRORLOG("Can't open log file %s.", log_name.data());
        return FileNotFound;
    }

    KeyType key;
    ValueType value;

    size_t total_size = ::lseek(fd, 0, SEEK_END);
    ::lseek(fd, 0, SEEK_SET);

    size_t read_size = 0;
    while (read_size < total_size) {
        unsigned int key_size;
        unsigned int value_size;
        char * key_buf = nullptr;
        char * value_buf = nullptr;
        size_t res = ::read(fd, &key_size, sizeof(unsigned int));
        if (res != sizeof(unsigned int) || key_size == 0)
            return IOError;
        read_size += sizeof(unsigned int);

        key_buf = new char[key_size];
        key.assign(key_buf, key_size);
        res = ::read(fd, key_buf, key_size);
        if (res != key_size)
            return IOError;
        read_size += key_size;

        res = ::read(fd, &value_size, sizeof(unsigned int));
        if (res != sizeof(unsigned int))
            return IOError;
        read_size += sizeof(unsigned int);
        // removed value?
        if ((value_size & 1<31) != 0) {
            // removed
            _mem->Delete(key);
            continue;
        }
        else {
            // read value
            value_buf = new char[value_size];
            value.assign(value_buf, value_size);
            res = ::read(fd, value_buf, value_size);
            if (res != value_size)
                return IOError;
            read_size += value_size;
            _mem->Put(key, value, true);
            continue;
        }
    }
    ::close(fd);
    ::remove(log_name.data());
    return Success;
}

} // namespace kv_engine