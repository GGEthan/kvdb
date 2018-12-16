#ifndef _ENGINE_TABLE_IO_H_
#define _ENGINE_TABLE_IO_H_

#include "MemTable.h"

#include "BaseType.h"

#include <vector>

namespace kv_engine {

class TableWriter {
public:
    TableWriter(MemTable * mem);

    ~TableWriter();

    Status Init(MemTable * mem);

    Status Init();

    Status WriteTable();

    static void WriteTableBackgroud(MemTable * mem);
private:
    int _fd = -1;
    
    MemTable * _mem = nullptr;

    std::vector<size_t> _index;

    size_t WriteRecord(const KeyType & key, const ValueType &value);


};

class TableReader {
public:
    TableReader(long id);


};

} // namespace kv_engine
#endif // _ENGINE_TABLE_IO_H_