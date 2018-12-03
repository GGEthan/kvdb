// SystemLog.cpp

#include <iostream>

#include <ctime>

#include <stdio.h>

#include <stdarg.h>

#include "SystemLog.h"

#include "Configuration.h"

namespace kv_base {

SystemLog* SystemLog::get_instance() {
    static SystemLog* sys_log = NULL;
    if (sys_log == NULL)
        sys_log = new SystemLog();
    return sys_log;
}

SystemLog::SystemLog() {
    // std::cout << "   ▄█   ▄█▄  ▄█    █▄  ████████▄  ▀█████████▄  " << std::endl
    //           << "  ███ ▄███▀ ███    ███ ███   ▀███   ███    ███ " << std::endl
    //           << "  ███▐██▀   ███    ███ ███    ███   ███    ███ " << std::endl
    //           << " ▄█████▀    ███    ███ ███    ███  ▄███▄▄▄██▀  " << std::endl
    //           << "▀▀█████▄    ███    ███ ███    ███ ▀▀███▀▀▀██▄  " << std::endl
    //           << "  ███▐██▄   ███    ███ ███    ███   ███    ██▄ " << std::endl
    //           << "  ███ ▀███▄ ███    ███ ███   ▄███   ███    ███ " << std::endl
    //           << "  ███   ▀█▀  ▀██████▀  ████████▀  ▄█████████▀  " << std::endl
    //           << "  ▀                                            " << std::endl;
    std::cout << "KVDB" << std::endl;
}

void SystemLog::set_log_file(string &file_path) {
    this->sys_log_file.open(file_path.c_str());
    if (!this->sys_log_file.is_open()) {
        // Can't open the system log file
        ERRORLOG("Can't open the system log file. System exiting...");
        EXIT();
    }
}

void SystemLog::persist() {
    if (!this->sys_log_file.is_open()) {
        // Can't user the syslog file but don't know why
        ERRORLOG("Can't write to the system log file. The SystemLog file is not open.");
    }
    else {
        // Write logs to the file
        DEBUGLOG("Pesisting system log.");
        std::list<std::pair<string, string> >::iterator iter = this->log_list.begin();
        while(iter!=this->log_list.end()) {
            this->sys_log_file << iter->first << " " << iter->second << "\n";
            iter++;
        }
        this->sys_log_file.flush();
        this->clear();
    }
}

void SystemLog::clear() {
    this->log_list.clear();
}
void SystemLog::log(const string& msg) {
    this->_log(msg.c_str(), _INFO);
}

void SystemLog::log(const char* format,...) {
    va_list args;
    va_start(args, format);    
    this->_log(format, args, _INFO);
    va_end(args);
}

void SystemLog::_log(const char* format, va_list args, const SysLogType &type) {
    char message[1024];
    if (!vsprintf(message, format, args)) {
        string msg = "Wrong log format: ";
        msg = msg + format;
        this->_log(msg.c_str(), _ERROR);
    }
    else {
        this->_log(message, type);
    }
}
void SystemLog::_log(const char* msg, const SysLogType &type) {
    // get local time
    time_t cur_time = time(NULL);
    tm* local = localtime(&cur_time);
    char ctime[30];
    sprintf(ctime,"%d-%02d-%02d %02d:%02d:%02d",
            local->tm_year+1900, local->tm_mon, local->tm_mday,
            local->tm_hour, local->tm_min, local->tm_sec);

    // save log message
    string stime(ctime);
    string message(msg);
    string type_message(this->TypeString[type]);

    std::pair<string, string> time_msg(stime, type_message + message);
    this->log_list.push_back(time_msg);

    // print log message
    std::cout << stime << " " << string(this->ColorString[type]) << message << "\n";
}

void DEBUGLOG(const string& msg) {
    if (!DEBUG)
        return;
    SystemLog* sys_log = SystemLog::get_instance();
    sys_log->_log(msg.c_str(), SystemLog::SysLogType::_DEBUG);
}

void DEBUGLOG(const char* format,...) {
    if (!DEBUG)
        return;
    SystemLog* sys_log = SystemLog::get_instance();
    va_list args;
    va_start(args, format);
    sys_log->_log(format, args, SystemLog::SysLogType::_DEBUG);
    va_end(args);
}

void INFOLOG(const string& msg) {
    SystemLog* sys_log = SystemLog::get_instance();
    sys_log->_log(msg.c_str(), SystemLog::SysLogType::_INFO);
}

void INFOLOG(const char* format,...) {
    SystemLog* sys_log = SystemLog::get_instance();
    va_list args;
    va_start(args, format);
    sys_log->_log(format, args, SystemLog::SysLogType::_INFO);
    va_end(args);
}
    
void WARNINGLOG(const string& msg) {
    SystemLog* sys_log = SystemLog::get_instance();
    sys_log->_log(msg.c_str(), SystemLog::SysLogType::_WARNING);
}

void WARNINGLOG(const char* format,...) {
    SystemLog* sys_log = SystemLog::get_instance();
    va_list args;
    va_start(args, format);
    sys_log->_log(format, args, SystemLog::SysLogType::_WARNING);
    va_end(args);
}

void ERRORLOG(const string& msg) {
    SystemLog* sys_log = SystemLog::get_instance();
    sys_log->_log(msg.c_str(), SystemLog::SysLogType::_ERROR);
}

void ERRORLOG(const char* format,...) {
    SystemLog* sys_log = SystemLog::get_instance();
    va_list args;
    va_start(args, format);
    sys_log->_log(format, args, SystemLog::SysLogType::_ERROR);
    va_end(args);
}
} // namespace kv_base