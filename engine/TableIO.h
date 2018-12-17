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

    static void CompactSSTable(int level);
private:
    int _fd = -1;
    
    MemTable * _mem = nullptr;

    std::vector<size_t> _index;

    size_t WriteRecord(const KeyType & key, const ValueType & value);


};

// TODO: use mmap
class TableReader {
public:
    TableReader(int level, long id);

    TableReader();

    ~TableReader();

    Status Init(int level, long id);

    Status Init();

    class Iterator {
    public:
        Iterator(TableReader* reader);
        
        Iterator(const TableReader::Iterator & that);

        bool next();

        void ReadRecord(KeyType & key, ValueType & value);

        void ReadKey(KeyType & key);
    private:
        TableReader* _reader = nullptr;

        off_t _offset = 0;

        KeyType _key;

        ValueType _value;
    };

    Status Find(const KeyType & key, ValueType & value);

    TableReader::Iterator Begin();

    friend class TableReader::Iterator;
private:
    int _fd = -1;

    int _level = -1;

    long _id = -1;

    off_t _index_offset = -1;

    size_t _size = -1;
    // NOT Safe !!!
    Status _ReadRecord(size_t offset, KeyType & key, ValueType & value);

    Status _ReadKey(size_t offset, KeyType & key);
    
    Status _BinarySearch();
};

} // namespace kv_engine
#endif // _ENGINE_TABLE_IO_H_