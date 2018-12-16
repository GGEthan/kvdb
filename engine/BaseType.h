#ifndef _ENGINE_BASE_TYPE_H_
#define _ENGINE_BASE_TYPE_H_

#include <string>

#include <string.h>

#include "Util.h"
namespace kv_engine {

using std::string;

class NoCopyString {
public:
	NoCopyString() { }
    NoCopyString(const char * s) : _data(s), _size(strlen(s)) { }
    NoCopyString(char * s, size_t n) : _data(s), _size(n) { } 
    NoCopyString(const string & s) : _data(s.data()), _size(s.size()) { }

	// The only copy method
	NoCopyString(const NoCopyString & that) {
		char* tmp = new char[that._size];
		_size = that._size;
		memcpy(tmp, that._data, _size);
		_data = tmp;
		_need_free = true;
	}

	~NoCopyString() {
		if (_need_free && _data != nullptr)
			delete _data;
	}

    const char * data() const { return _data;}
    
    unsigned int size() const { return _size;}

	int compare(const NoCopyString & that) const{
		if (that._size == _size)
			return memcmp(_data, that._data, _size);
		if (that._size < _size) {
			if (memcmp(_data, that._data, that._size) >= 0)
				return 1;
			return -1;
		}
		if (memcmp(_data, that._data, _size) > 0)
			return 1;
		return -1;
	}


	bool operator == (const NoCopyString & that) const {
		return compare(that) == 0;
	}

	bool operator < (const NoCopyString & that) const {
		return compare(that) < 0;
	}

protected:
    const char * _data = (const char *)this;
    unsigned int _size = 0;
	bool _need_free = false;
};

typedef NoCopyString KeyType;

class ValueType : public NoCopyString {
public :
	bool removed = false;
	ValueType(){ }
	ValueType(const ValueType & that) : removed(that.removed), NoCopyString(that){ }
};



} // namespace kv_engine
#endif // _ENGINE_BASE_TYPE_H_