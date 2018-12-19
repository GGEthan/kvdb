#include "Meta.h"

#include "Configuration.h"

#include <unistd.h>

#include <fcntl.h>

#include <list>

#include <errno.h>

#include <string.h>

namespace kv_engine {

Meta::Iterator Meta::Begin() {
    return Meta::Iterator(this);
}

Meta::Iterator Meta::Level(int _level) {
    return Meta::Iterator(this, _level);
}

long Meta::new_id() {
    return _id_count++;
}

SSTABLE_INFO * Meta::Iterator::next() {
    // TODO : thread safe
    if (parent->sizes[level] <= target) {
        level ++;
        target = 0;
    }
    if (parent->sizes[level] == 0)
        return nullptr;

    return &(parent->sstable_info[level][target++]);
}

Status Meta::Deserialize(const char * path) {
    int fd = open(path, O_RDWR | O_CREAT, 0777);
    if (fd <= 0) {
        ERRORLOG("Can't open meta file %s.", path);
        return FileNotFound;
    }
    _fd = fd;
    if (lseek(fd, 0, SEEK_END) == 0) {
        INFOLOG("Create Meta file %s", path);
        _Persist();
        return Success;
    }
    int res = pread(_fd, this, sizeof(Meta), 0);
    if (res != sizeof(Meta)) {
        ERRORLOG("Read meta file %s fail. [%d]", path, res);
        return IOError;
    }
    _fd = fd;
    return Success;
}

Status Meta::_Persist() {
    if (_fd <= 0) {
        ERRORLOG("Can't persist meta file.[File not open]");
        return FileNotFound;
    }
    int res = pwrite(_fd, this, sizeof(Meta), 0);
    if (res != sizeof(Meta)) {
        ERRORLOG("Can't persist meta file.[size %d]", res);
        ERRORLOG("%s", strerror(errno));
        return IOError;
    }
    return Success;
}

Status Meta::Persist() {
    return _Persist();
}

Status Meta::NewSSTable(long id, int level) {
    string file_name = ConcatFileName(Configuration::DATA_DIR, Configuration::SSTABLE_NAME, level, id);
    if (Configuration::TableReaderMap.find(id) == Configuration::TableReaderMap.end()) {
        TableReader * new_reader = new TableReader(level, id);
        new_reader->Init();
        Configuration::TableReaderMap[id] = new_reader;
    }
    sstable_info[level][sizes[level]].id = id;
    sstable_info[level][sizes[level]].level = level;
    sizes[level] ++;
    _Persist();
    if (sizes[level] >= 100) {
        ERRORLOG("Full SSTables!!!!! level%d, size%d", level, sizes[level]);
        return UnknownError;
    }
    return Success;
}

Status Meta::Compact(int old_level, long new_id) {
    if (sizes[old_level] < Configuration::COMPACT_SIZE) {
        ERRORLOG("Meta::Compact size[%d]=%d", old_level, sizes[old_level]);
        return UnknownError;
    }
    std::list<long> old_ids;
    for (int i = 0; i < Configuration::COMPACT_SIZE; i++)
        old_ids.push_back(sstable_info[old_level][i].id);
    
    for (int i = Configuration::COMPACT_SIZE; i < sizes[old_level]; i++) 
        sstable_info[old_level][i - Configuration::COMPACT_SIZE] = sstable_info[old_level][i];
    
    sizes[old_level] -= Configuration::COMPACT_SIZE;
    sstable_info[old_level + 1][sizes[old_level + 1]] = SSTABLE_INFO(new_id, old_level + 1);
    sizes[old_level + 1] ++;

    if (old_level == levels)
        levels ++;
    _Persist();
    
    for(long old_id : old_ids) {
        TableReader * old_reader = Configuration::TableReaderMap[old_id];
        Configuration::TableReaderMap.erase(old_id);
        old_reader->Remove();
        delete old_reader;
    }

    return Success;
}






} // namespace kv_engine