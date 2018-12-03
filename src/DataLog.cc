// DataLog.cpp

#include "DataBase.h"

#include <string.h>

namespace kv_base {

LogEntry::LogEntry() {

}
LogEntry::LogEntry(LogEntry::Operation _op, DataNode* data) {
    this->op = _op;
    switch (_op) {
        case write: {
            this->target.copy(data);
            break;
        }
        case update: {
            this->target.copy(data);
            break;
        }
        case remove: {
            this->target.key = new char[strlen(data->key)];
            this->target.key = strcpy(this->target.key, data->key);
            break;
        }
    }
}

int LogEntry::serialize(char* buf) {
    int total = 0;
    memcpy(buf + total, &(this->op), sizeof(this->op));
    total += sizeof(this->op);
    switch (this->op) {
        case remove: {
            // [operation(4 Byte)]+[key(m Byte)]
            strcpy(buf + total, this->target.key);
            total += strlen(this->target.key) + 1;
            break;
        }
        case update: 
        case write:{
            // [operation(4 Byte)]+[key(m Byte)]+[vtype(1 Byte)]+[vsize(4 Byte)]+[value(n Byte)]
            strcpy(buf + total, this->target.key);
            total += strlen(this->target.key) + 1;
            memcpy(buf + total, &(this->target.vtype), sizeof(this->target.vtype));
            total += sizeof(this->target.vtype);
            memcpy(buf + total, &(this->target.vsize), sizeof(this->target.vsize));
            total += sizeof(this->target.vsize);
            memcpy(buf + total, this->target.value, this->target.vsize);
            total += this->target.vsize;
            break;
        }
    }
    return total;
}

int LogEntry::deserialize(char* buf) {
    // operation
    int total = 0;
    memcpy(&(this->op), buf + total, sizeof(this->op));
    total += sizeof(this->op);
    
    // key
    int len = strlen(buf + total) + 1;
    this->target.key = new char[len];
    strcpy(this->target.key, buf + total);
    total += len;

    // value
    if (this->op != remove) {
        memcpy(&(this->target.vtype), buf + total, sizeof(this->target.vtype));
        total += sizeof(this->target.vtype);
        memcpy(&(this->target.vsize), buf + total, sizeof(this->target.vsize));
        total += sizeof(this->target.vsize);
        this->target.value = new char[this->target.vsize];
        memcpy(this->target.value, buf + total, this->target.vsize);
        total += this->target.vsize;
    }
    return total;
}

void LogEntry::print(std::ostream &out) {
    switch (this->op) {
        case Operation::remove:{
            out << "op:remove ";
            out << "Key:" << this->target.key;
            break;
        }
        case Operation::update:{
            out << "op:update ";
            this->target.print(out);
            break;
        }
        case Operation::write:{
            out << "op:write ";
            this->target.print(out);
            break;
        }
    }
}

DataLog* DataLog::get_instance() {
    static DataLog* log = nullptr;
    if (log == nullptr)
        log = new DataLog();
    return log;
}
DataLog::DataLog() {
    // nothing to do here
}

int DataLog::write(DataNode* data) {
    this->log_list.push_back(new LogEntry(LogEntry::Operation::write, data));
    return 1;
}

int DataLog::remove(DataNode* data) {
    this->log_list.push_back(new LogEntry(LogEntry::Operation::remove, data));
    return 1;
}

int DataLog::update(DataNode* data) {
    this->log_list.push_back(new LogEntry(LogEntry::Operation::update, data));
    return 1;
}

int DataLog::init(const string &log_path) {
    // only use once
    static bool is_init = false;
    if (is_init) 
        return 0;
    is_init = true;
    return this->__init(log_path);
}

int DataLog::__init(const string &log_path) {
    std::ifstream in(log_path.c_str());
    char buf[1024];
    int total = 0;
    INFOLOG("Initializing DataLog from file %s", log_path.c_str());
    if (!in.is_open()) {
        // Log file is not open
        WARNINGLOG("Can't open the DataLog file %s", log_path.c_str());
        WARNINGLOG("Creating DataLog file %s", log_path.c_str());
        // Create the Log file
        std::fstream temp_file(log_path.c_str(), std::ios::out);
        if (!temp_file.is_open()) {
            ERRORLOG("Can't create DataLog file %s", log_path.c_str());
            EXIT();
        } else 
            temp_file.close();
    } else {
        // init data log
        while (in.read((char*)&total, sizeof(total))) {
            if (total <= 0) {
                ERRORLOG("Initializing Error. Wrong LogEntry format");
                continue;
            }
            if (!in.read(buf, total)) {
                ERRORLOG("Initializing Error. Missing Log.");
                continue;
            }
            LogEntry* new_entry = new LogEntry();
            if (new_entry->deserialize(buf) != total) {
                DEBUGLOG("Format with wrong size");
            }
            this->log_list.push_back(new_entry);
        }
    }
    in.close();
    this->log_file.open(log_path.c_str());
    return 1;
}

int DataLog::__persist() {
    INFOLOG("Persisting the DataLog");
    if (!this->log_file.is_open()) {
        ERRORLOG("Can't persist the DataLog. The Log File is not open.");
        return 0;
    }
    char buf[1024];
    int total = 0;
    auto iter = this->log_list.begin();
    while (iter != this->log_list.end()) {
        total = (*iter)->serialize(buf);
        if (total <= 0) {
            DEBUGLOG("Serialize LogEntry Error");
        } else {
            this->log_file.write((char*)&total, sizeof(total));
            this->log_file.write(buf, total);
        }
        iter++;
    }
    this->log_file.flush();
    this->log_file.close();
    return 1;
}

int DataLog::persist() {
    return this->__persist();
}

void DataLog::print_all_entries(std::ostream &out) {
    auto iter = this->log_list.begin();
    while (iter != this->log_list.end()) {
        (*iter)->print(out);
        out << "\n";
        iter++;
    }
}
} // namespace kv_base