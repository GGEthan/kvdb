#include "DBLog.h"

#include "Configuration.h"

#include <fcntl.h>

#include <unistd.h>

namespace kv_engine {

DBLog::~DBLog() {
    // delete log file here
    
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

Status DBLog::Open(const long & _id) {
    string log_name = ConcatFileName(Configuration::DBLOG_NAME, 0, _id);
    int fd = ::open(log_name.data(), O_WRONLY | O_CREAT, 0777);
    if (fd <= 0) {
        ERRORLOG("Can't create log file %s.", log_name.data());
        return FileNotFound;
    }    
    _fd = fd;
    return Success;
}

} // namespace kv_engine