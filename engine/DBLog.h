#ifndef _ENGINE_DBLOG_H_
#define _ENGINE_DBLOG_H_

#include "BaseType.h"

#include "Util.h"

#include "MemTable.h"
namespace kv_engine {

class DBLog {
public:
    Status Log(const KeyType & key, const ValueType & value);
    
    Status Open(long _id);

    Status Recover(MemTable * _mem);
    
private:
    int _fd = -1; // log file

    long id;
};

}
#endif // _ENGINE_DBLOG_H_