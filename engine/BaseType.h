#ifndef _ENGINE_BASE_TYPE_H_
#define _ENGINE_BASE_TYPE_H_

#include <string>

#include <string.h>

namespace kv_engine {

using std::string;

enum Status {
    Success,
    KeyNotFound,
    TableFull,
    IOError,
    UnknownError
};

class NoCopyString {
public:
    NoCopyString(const char * s) : _data(s), _size(strlen(s)) { }
    NoCopyString(char * s, size_t n) : _data(s), _size(n) { } 
    NoCopyString(const string & s) : _data(s.data()), _size(s.size()) { }

    const char * data() const { return _data;}
    
    int size() const { return _size;}

protected:
    const char * _data = nullptr;
    size_t _size = 0;
};

typedef NoCopyString KeyType;

typedef NoCopyString ValueType;

class ScanHandle {

};

} // namespace kv_engine
#endif // _ENGINE_BASE_TYPE_H_