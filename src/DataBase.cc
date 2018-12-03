// DataBase.cpp

#include <iostream>

#include "DataBase.h"

#include "SystemLog.h"

namespace kv_base {

DataBase* DataBase::get_instance() {
    static DataBase* db = nullptr;
    if (db == nullptr) 
        db = new DataBase();
    return db;
}

DataBase::DataBase() {
    // nothing to do here
    this->data_log = DataLog::get_instance();
}

// TODO: multi-thread control
int DataBase::read(const string &key, DataNode &node) {
    return this->__read(key, node);
}
int DataBase::write(const string &key, const char* value, const type &vtype, const int &size) {
    return this->__write(key, value, vtype, size);
}
int DataBase::remove(const string &key) {
    return this->__remove(key);
}
int DataBase::update(const string &key, const char* value, const type &vtype, const int &size) {
    return this->__update(key, value, vtype, size);
}


int DataBase::__read(const string &key, DataNode &node) {
    // Key doesn't exist
    if (this->data.find(key) == this->data.end()) {
        DEBUGLOG("Reading an unexisted key: %s.", key.c_str());
        return 0;
    }
    DataNode* target = this->data[key];
    node.copy(target);
    return 1;
}

int DataBase::__write(const string &key, const char* value, const type &vtype, const int &vsize) {
    // Key exists
    if (this->data.find(key) != this->data.end()) {
        DEBUGLOG("Writing an existed key: %s", key.c_str());
        return 0;
    }
    // create new node
    DataNode* new_node = new DataNode(key.c_str(), value, vtype, vsize);
    // update log first
    this->data_log->write(new_node);
    // update data
    this->data[key] = new_node;
    return 1;
}

int DataBase::__remove(const string &key) {
    // Key doesn't exist
    if (this->data.find(key) == this->data.end()) {
        DEBUGLOG("Removing an unexisted key: %s", key.c_str());
        return 0;
    }
    DataNode* target = this->data[key];
    // log first
    this->data_log->remove(target);
    // delete data
    this->data.erase(key);
    // release node
    delete target;
    return 1;
}

int DataBase::__update(const string &key, const char* value, const type &vtype, const int &size) {
    // Key doesn't exist
    if (this->data.find(key) == this->data.end()) {
        DEBUGLOG("Updating an unexisted key: %s", key.c_str());
        return 0;
    }
    DataNode* old_node = this->data[key];
    // create new node
    DataNode* new_node = new DataNode(key.c_str(), value, vtype, size);
    // log first
    this->data_log->update(new_node);
    // update index
    this->data[key] = new_node;
    // release old node
    delete old_node;
    return 1;
}

int DataBase::init(const string &data_file) {
    // use this method only once
    static bool is_init = false;
    if (is_init)
        return 0;
    is_init = true;
    return this->__init(data_file);
}

int DataBase::__init(const string &data_file) {
    std::ifstream in(data_file.c_str());
    char buf[1024];
    int total = 0;
    INFOLOG("Initializing DataBase from file %s", data_file.c_str());
    if (!in.is_open()) {
        // Data file is not open
        WARNINGLOG("Can't open the DataBase file %s", data_file.c_str());
        WARNINGLOG("Creating DataBase file %s", data_file.c_str());
        // Create the Data file here
        std::fstream temp_file(data_file.c_str(), std::ios::out);
        if (!temp_file.is_open()) {
            ERRORLOG("Can't create the file %s.", data_file.c_str());
            EXIT();
        } else 
            temp_file.close();
    }
    else {
        while (in.read((char*)&total, sizeof(total))) {
            if (total <= 0) {
                ERRORLOG("Initializing Error. Wrong data file format.");
                continue;
            }
            if (!in.read(buf, total)) {
                ERRORLOG("Initializing Error. Missing Data.");
                continue;              
            }
            DataNode* new_node = new DataNode();
            if (new_node->deserialize(buf) != total) {
                DEBUGLOG("Format with wrong size");
            }
            this->data.insert(std::pair<string, DataNode*>(
                string(new_node->key),
                new_node
            ));
        }
    }
    in.close();
    this->db_file.open(data_file.c_str());
    return 1;
}
int DataBase::persist() {
    return this->__persist();
}
int DataBase::__persist() {
    INFOLOG("Persisting the DataBase");
    if (!this->db_file.is_open()) {
        ERRORLOG("Can't persist the DataBase. The Data File is not open.");
        return 0;
    }
    char buf[1024];
    int total = 0;
    auto iter = this->data.begin();
    while (iter != this->data.end()) {
        total = iter->second->serialize(buf);
        if (total <= 0) {
            DEBUGLOG("Serialize DataNode Error");
        } else {
            this->db_file.write((char*)&total, sizeof(total));
            this->db_file.write(buf, total);
        }
        iter++;
    }
    this->db_file.flush();
    this->db_file.close();
    return 1;
}

int DataBase::recover() {
    // use this method only once
    static bool is_recover = false;
    if (is_recover)
        return 0;
    is_recover = true;
    return this->__recover();
}

int DataBase::__recover() {
    INFOLOG("Recovering the DataBase");

    auto iter = this->data_log->log_list.begin();
    while (iter != this->data_log->log_list.end()) {
        if (!this->__read_log(*iter)) {
            this->print_all_nodes(std::cout); 
            ERRORLOG("Fail to read the LogEntry");
        }
        else {
        // stupidly use this->__method() will add a new LogEntry
            this->data_log->log_list.pop_back();
        }
        iter ++;

    }
    return 1;
}

int DataBase::__read_log(const LogEntry* log) {
    switch (log->op) {
        case LogEntry::Operation::remove: {
            return this->__remove(string(log->target.key));
            break;
        }
        case LogEntry::Operation::update: {
            return this->__update(
                string(log->target.key),
                log->target.value,
                log->target.vtype,
                log->target.vsize
            );
            break;
        }
        case LogEntry::Operation::write: {
            return this->__write(
                string(log->target.key),
                log->target.value,
                log->target.vtype,
                log->target.vsize
            );
            break;
        }
    }
    ERRORLOG("Can't parse the Operation of the LogEntry %d", log->op);
    return 0;
}

void DataBase::print_all_nodes(std::ostream &out) {
    auto iter = this->data.begin();
    while (iter != this->data.end()) {
        iter->second->print(out);
        out << '\n';
        iter ++;
    }
}

} // namespace kv_base
