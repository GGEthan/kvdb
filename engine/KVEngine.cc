#include "KVEngine.h"

#include "MemTable.h"

#include "Configuration.h"

#include <dirent.h>

#include <sys/stat.h>

#include <list>
namespace kv_engine {
using std::string;

KVEngine::~KVEngine() {
    MemTable * _mem = MemTable::memTable;
    MemTable::memTable = nullptr;

    TableWriter::WriteTableBackgroud(_mem);
    INFOLOG("886");
}
Status KVEngine::Open(const std::string & conf_path) {
    return UnknownError;
}

Status KVEngine::Open(const std::string & log_dir, const std::string & data_dir) {

    Configuration::LOG_DIR = log_dir;
    Configuration::DATA_DIR = data_dir;
    Configuration::meta = new Meta();
    // Open data file
    DIR* _data_dir = opendir(data_dir.data());
    if (_data_dir == NULL) {
        if (::mkdir(data_dir.data(), 0777) != 0) {
            ERRORLOG("Can't create dir %s", data_dir.data());
            return FileNotFound;
        }
    } else {
        // search data file
        dirent * entry;
        while ((entry = readdir(_data_dir)) != nullptr) {
            string file_name(entry->d_name);
            int level;
            long id;
            string file_head;
            if (SplitFileName(file_name, file_head, level, id) != Success)
                continue;
            if (file_head != Configuration::SSTABLE_NAME)
                continue;
            TableReader* new_reader = new TableReader(level, id);
            if (new_reader->Init() != Success)
                return FileNotFound;
            Configuration::TableReaderMap[id] = new_reader;
        }
    }
    // recover meta 
    Configuration::meta->Deserialize((data_dir + "/meta").data());
    // log
    // create MemTbale
    DIR* _log_dir = opendir(log_dir.data());
    if (_log_dir == NULL) {
        if (::mkdir(log_dir.data(), 0777) != 0) {
            ERRORLOG("Can't create dir %s", log_dir.data());
            return FileNotFound;
        }
        // create MemTbale
        MemTable::memTable = new MemTable();
    } else {
        // create MemTbale
        MemTable::memTable = new MemTable();
        // search log file
        dirent * entry;
        std::list<std::string> entry_list;
        while ((entry = readdir(_log_dir)) != nullptr) {
            string entry_name(entry->d_name);
            entry_list.push_back(entry_name);
        }
        closedir(_log_dir);

        for(auto iter:entry_list) {
            int level;
            long id;
            string file_head;
            if (SplitFileName(iter, file_head, level, id) != Success)
                continue;
            if (file_head != Configuration::DBLOG_NAME) 
                continue;
            // recover log file
            DBLog::Recover(MemTable::memTable, id);
            if (MemTable::memTable->ApproximateMemorySize() >= Configuration::MAX_MEMTABLE_SIZE)
                MemTable::memTable->SafeSetImmutable();
        }
    }

    return Success;
}


Status KVEngine::Put(const KeyType & key, const ValueType & value, const bool overwrite) {
    // Test if the memtable is writable
    if (MemTable::memTable->ApproximateMemorySize() >= Configuration::MAX_MEMTABLE_SIZE)
        MemTable::memTable->SafeSetImmutable();
    
    ASSERT(MemTable::memTable->Put(key, value, overwrite));
    
    return Success;
}

Status KVEngine::Get(const KeyType & key, ValueType & value) {
    Meta * meta = Configuration::meta;
    Meta::Iterator iter = meta->Begin();
    // read memTable first
    Status res = MemTable::memTable->Get(key, value);
    if (res == Success) {
        if (value.removed)
            return KeyNotFound;
        return Success;
    }

    // read ImmutMemTable
    if (MemTable::immutableTable != nullptr) {
        res = MemTable::immutableTable->Get(key, value);
        if (res == Success) {
            if (value.removed)
                return KeyNotFound;
            return Success;
        }
    }

    // read SSTables
    SSTABLE_INFO* info = nullptr;
    while((info = iter.next()) != nullptr) {
        auto find = Configuration::TableReaderMap.find(info->id);
        if (find == Configuration::TableReaderMap.end())
            continue;
        TableReader * reader = find->second;
        if (reader == nullptr)
            continue;
        res = reader->Find(key, value);
        if (res == Success) {
            if (value.removed)
                return KeyNotFound;
            return Success;
        }
    }

    return KeyNotFound;
}

Status KVEngine::Scan(const KeyType & start, const int record_count, ScanHandle & handle) {
    return UnknownError;
}

Status KVEngine::Delete(const KeyType & key) {
    // Test if the memtable is writable
    if (MemTable::memTable->ApproximateMemorySize() >= Configuration::MAX_MEMTABLE_SIZE)
        MemTable::memTable->SafeSetImmutable();
    
    ASSERT(MemTable::memTable->Delete(key));

    return Success;
}

} // namespace kv_engine