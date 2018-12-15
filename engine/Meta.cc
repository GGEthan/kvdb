#include "Meta.h"

#include "Configuration.h"

#include <unistd.h>

#include <fcntl.h>

namespace kv_engine {

Meta::Iterator Meta::Begin() {
    return Meta::Iterator(this);
}

Meta::Iterator Meta::Level(int _level) {
    return Meta::Iterator(this, _level);
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
    int fd = open(path, O_RDWR);
    if (fd <= 0) {
        ERRORLOG("Can't open meta file %s.", path);
        return FileNotFound;
    }
    _fd = fd;
    int res = read(_fd, this, sizeof(Meta));
    if (res != sizeof(Meta)) {
        ERRORLOG("Read meta file %s fail. [%d]", path, res);
        return IOError;
    }
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
        return IOError;
    }
    return Success;
}

Status Meta::Persist() {
    return _Persist();
}

Status Meta::NewSSTable(long id, int level) {
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
    for (int i = Configuration::COMPACT_SIZE; i < sizes[old_level]; i++)
        sstable_info[old_level][i - Configuration::COMPACT_SIZE] = sstable_info[old_level][i];
    sizes[old_level] -= Configuration::COMPACT_SIZE;
    sizes[old_level + 1] ++;
    sstable_info[old_level + 1][sizes[old_level + 1]] = SSTABLE_INFO(new_id, old_level + 1);
    if (old_level == levels)
        levels ++;
    _Persist();
}






} // namespace kv_engine