#ifndef _ENGINE_DBLOG_H_
#define _ENGINE_DBLOG_H_

#include "BaseType.h"

#include "Util.h"

#include <mutex>

namespace kv_engine {
using std::mutex;

class MemTable;

class DBLog {
public:
    DBLog(){}

    ~DBLog();

    Status Log(const KeyType & key, const ValueType & value);
    
    Status Open(const long & _id);

    static Status Recover(MemTable * _mem, long id);
    
private:
    int _fd = -1; // log file

    long id;

    size_t _size = 0;
    
    mutex _size_mtx;

};

}
#endif // _ENGINE_DBLOG_H_