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
	NoCopyString(const bool & _removed) : removed(_removed) { }
	// copy method
	NoCopyString(const NoCopyString & that) {
		copy(that);
	}
	// copy method
	NoCopyString& operator=(const NoCopyString & that) {
		copy(that);
		return *this;
	}
	// copy method
	void copy(const NoCopyString & that) {
		if (_need_free)
			delete _data;
		char* tmp = new char[that._size];
		_size = that._size;
		memcpy(tmp, that._data, _size);
		_data = tmp;
		_need_free = true;
		removed = that.removed;
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

	void replace(const NoCopyString & that) {
		if (_need_free)
			delete _data;
		_data = that._data;
		_size = that._size;
		_need_free = false;
		removed = that.removed;
	}

	void assign(const char * src, size_t n) {
		if (_need_free)
			delete _data;
		_data = src;
		_size = n;
		_need_free = true;
	}
	bool removed = false;
protected:
    const char * _data = (const char *)this;
    unsigned int _size = 0;
	bool _need_free = false;
};

typedef NoCopyString KeyType;
typedef NoCopyString ValueType;
// class ValueType : public NoCopyString {
// public :
	
// 	ValueType(){removed = false;}
// 	ValueType(const ValueType & that) : removed(that.removed), NoCopyString(that){ }
// 	ValueType(const string & str) : removed(false), NoCopyString(str) { }
// 	ValueType(const bool & _removed) : removed(_removed) { }
// 	ValueType(const char * str) : removed(false), NoCopyString(str) { }
// };

class ScanHandle {
public:
    ScanHandle(){}
    virtual ~ScanHandle(){}

    virtual Status GetKeyValue(KeyType & key, ValueType & value) = 0;

    virtual bool GetNext() = 0;
};

} // namespace kv_engine
#endif // _ENGINE_BASE_TYPE_H_