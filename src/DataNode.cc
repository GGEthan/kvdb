// DataNode.cpp

#include "DataBase.h"

#include <sstream>

#include <string.h>

namespace kv_base {

DataNode::DataNode() {
    // Nothing to do here
    this->key = nullptr;
    this->value = nullptr;
}

DataNode::DataNode(const char* k, const char* v, const type &_vtype, const int &_size) {
    this->vtype = _vtype;
    this->key = new char[strlen(k) + 1];
    this->key = strcpy(this->key, k);
    if (_vtype < STRING) 
        // basic type
        this->vsize = size_map[_vtype];
    else if (_vtype == STRING)
        // string
        this->vsize = strlen(v) + 1;
    else 
        // list, must use _size
        this->vsize = _size;
    
    this->value = new char[this->vsize];
    memcpy(this->value, v, this->vsize);
}

DataNode::~DataNode() {
    delete this->key;
    delete this->value;
}

void DataNode::copy(DataNode* src) {
    this->vtype = src->vtype;
    this->vsize = src->vsize;
    // strlen + '\0'
    this->key = new char[strlen(src->key) + 1];
    this->key = strcpy(this->key, src->key);
    this->value = new char[this->vsize];
    memcpy(this->value, src->value, this->vsize);
}

int DataNode::serialize(char* buf) {
    int total = 0;
    // key
    int len = strlen(this->key) + 1;
    memcpy(buf + total, this->key, len);
    total += len;
    // vtype
    memcpy(buf + total, &(this->vtype), sizeof(this->vtype));
    total += sizeof(this->vtype);
    // vsize
    memcpy(buf + total, &(this->vsize), sizeof(this->vsize));
    total += sizeof(this->vsize);
    //value
    memcpy(buf + total, this->value, this->vsize);
    total += this->vsize;

    return total;
}

int DataNode::deserialize(char* buf) {
    this->clear();
    int total = 0;
    // key
    int key_len = strlen(buf) + 1;
    this->key = new char[key_len];
    memcpy(this->key, buf + total, key_len);
    total += key_len;
    // vtype
    memcpy(&(this->vtype), buf + total, sizeof(this->vtype));
    total += sizeof(this->vtype);
    // vsize
    memcpy(&(this->vsize), buf + total, sizeof(this->vsize));
    total += sizeof(this->vsize);
    // value
    this->value = new char[this->vsize];
    memcpy(this->value, buf + total, this->vsize);
    total += this->vsize;
    return total;
}

void DataNode::clear() {
    if (this->key != nullptr) {
        delete this->key;
        this->key = nullptr;
    }
    if (this->value != nullptr) {
        delete this->value;
        this->value = nullptr;
    }
}

void DataNode::format_value(std::ostream &ss, type t, const char* src) {
    switch (t) {
        case INT32:{ss << *((int*)src); break;}
        case UNSIGNED_INT32:{ss << *((unsigned int*)src); break;}
        case INT64:{ss << *((long long int*)src); break;}
        case UNSIGNED_INT64:{ss << *((unsigned long long int*)src); break;}
        case SHORT:{ss << *((short*)src); break;}
        case UNSIGNED_SHORT:{ss << *((unsigned short*)src); break;}
        case FLOAT:{ss << *((float*)src); break;}
        case DOUBLE:{ss << *((double*)src); break;}
        case CHAR:{ss << *((char*)src); break;}
        case STRING:{ss << src; break;}
    }
}

void DataNode::print(std::ostream &sstream) {
    // std::stringstream sstream;
    sstream << "Key:" << this->key;
    sstream << " Size:" << this->vsize;
    if (this->vtype < LIST) {
        sstream << " Type:" << this->name_map[this->vtype];
        sstream << " Value:";
        this->format_value(sstream, this->vtype, this->value);
    }
    else {
        type t = this->vtype - LIST;
        sstream << " Type:List[" << this->name_map[t] << "]";
        sstream << " Value:[";
        for (int i = 0; i < this->vsize;) {
            this->format_value(sstream, t, this->value + i);
            if (t == STRING)
                i += strlen(this->value + i) + 1;
            else 
                i += size_map[t];
            if (i < this->vsize)
                sstream << ',';
            else
                sstream << ']';
        }
    }
}

} // namespace kv_base