// DataBase.h
#ifndef DATABASE_H
#define DATABASE_H



#include "common.h"

#include "Configuration.h"


// define data type here
#define INT32 0
#define UNSIGNED_INT32 1
#define INT64 2
#define UNSIGNED_INT64 3
#define SHORT 4
#define UNSIGNED_SHORT 5
#define FLOAT 6
#define DOUBLE 7
#define CHAR 8
#define STRING 9
// any type >= 10 should be a LIST
// for example, list<short> is 14 list<char> is 18
#define LIST 10 

namespace kv_base {

typedef char type; // use one Byte to mark the type of the value

// DataNode actually saves the k & v,
class DataNode {
public:
    char* key;
    char* value;
    type vtype;
    int vsize; 
    DataNode();
    DataNode(const char* k, const char* v, const type &_vtype, const int &_size);
    ~DataNode();
    void copy(DataNode* source);
    int serialize(char* buf);
    int deserialize(char* buf);

    // functions for test
    void print(std::ostream &sstream);
    void format_value(std::ostream &sstream, type t, const char* src);
private:
    int size_map[9] = {
        4, // int32
        4, // unsigned int32
        8, // int64
        8, // unsigned int64
        2, // short
        2, // unsigned short
        4, // float
        8, // double
        1 // char
    };
    const char* name_map[10] = {
        "int32",
        "unsigned int32",
        "int64",
        "unsigned int64",
        "short",
        "unsigned short",
        "float",
        "double",
        "char",
        "string"
    };
    void clear();
};

class LogEntry {
public:
    enum Operation {write, update, remove} op;
    DataNode target;
    LogEntry();
    LogEntry(Operation _op, DataNode* data);
    int serialize(char* buf);
    int deserialize(char* buf);

    // functions for test
    void print(std::ostream &out);
};

// DataLog 
class DataLog {
public:
    friend class DataBase;
    static DataLog* get_instance();
    // There are 3 operations that should write the DataLog,
    // which are 'write', 'update', 'remove'.
    // For every opertation, we just record the pointer of the
    // changed DataNode. As for 'remove', DataBase should set the
    // value pointer as nullptr.
    // Particularly, when a DataNode has been changed more than once,
    // we just record the latest value. Because the olders will eventually
    // override anyway.
    int write(DataNode* data);
    int remove(DataNode* data);
    int update(DataNode* data);

    int init(const string &log_path);

    int persist();

    // functions for test
    void print_all_entries(std::ostream &out);
private:
    DataLog();
    // record the data log
    std::list<LogEntry*> log_list;
    std::ofstream log_file;
    int __init(const string &log_path);
    int __persist();
};

// DataBase is a singleton class.
// It is the only object that can manipulate data.
// It provides thread-safe CRUD methods for
// multi-thread IO request.
class DataBase {
public:
    static DataBase* get_instance();
    
    // initialize the DataBase
    int init(const string &data_file);

    // persist
    int persist();
    
    // recover from DataLog
    int recover();

    // thread-safe CRUD
    int read(const string &key, DataNode &node);
    int write(const string &key, const char* value, const type &vtype, const int &size);
    int remove(const string &key);
    int update(const string &key, const char* value, const type &vtype, const int &size);

    // functions for test
    void print_all_nodes(std::ostream &out);
private:
    std::map<string, DataNode*> data; // data base index
    DataLog* data_log;
    std::ofstream db_file;
    DataBase();
    // thread-unsafe CRUD
    int __read(const string &key, DataNode &node);
    int __write(const string &key, const char* value, const type &vtype, const int &size);
    int __remove(const string &key);
    int __update(const string &key, const char* value, const type &vtype, const int &size);

    int __init(const string &data_file);
    int __persist();
    int __recover();
    int __read_log(const LogEntry* log);
};

} // namespace kv_base
#endif // DATABASE_H