#ifndef _ENGINE_META_H_
#define _ENGINE_META_H_

#include "BaseType.h"
namespace kv_engine {

struct SSTABLE_INFO {
    long id = 0;
    
    int level = 0;
    
    SSTABLE_INFO(long _id, int _level) : id(_id), level(_level) {}
    SSTABLE_INFO() {}
    void operator = (const SSTABLE_INFO & that) {
        id = that.id;
  
        level = that.level;
    }
};

// simple implement
class Meta {
public:
    Meta(){ }
    Status NewSSTable(long id, int level);
    
    Status Compact(int old_level, long new_id);

    Status Deserialize(const char * path);
    
    Status Persist();
   
    class Iterator {
    public:
        Iterator(Meta* p) : parent(p) {}
        
        Iterator(Meta* p, int _level) : parent(p), level(_level) {}
        
        SSTABLE_INFO* next();
    private:
        Meta* parent;
        
        int level = 0;
        
        int target = 0;
    };

    Meta::Iterator Begin();

    Meta::Iterator Level(int _level);

    int size(int _level) { return sizes[_level];}

    SSTABLE_INFO* FirstTable(int _level) {return sstable_info[_level];}

    long new_id();

    friend class Meta::Iterator;
private:
    SSTABLE_INFO sstable_info[100][100];
    
    int sizes[100];
    
    int levels = 0;

    int _fd = -1;
   
    long _id_count = 0;

    Status _Persist();
};

} // namespace kv_engine
#endif // _ENGINE_META_H_
