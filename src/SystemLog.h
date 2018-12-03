// SystemLog.h

#ifndef SYSTEMLOG_H
#define SYSTEMLOG_H

#include <list>

#include "common.h"

namespace kv_base {
using std::string;
// SystemLog is a singleton class (or object).
// It is the only object that can use stdout.
// Using log() to print messages in the console and
// the system log file.
class SystemLog {
public:

    friend void INFOLOG(const string& msg);
    friend void INFOLOG(const char* format,...);
    friend void DEBUGLOG(const string& msg);
    friend void DEBUGLOG(const char* format,...);
    friend void WARNINGLOG(const string& msg);
    friend void WARNINGLOG(const char* format,...);
    friend void ERRORLOG(const string& msg);
    friend void ERRORLOG(const char* format,...);

    // Create or get the SystemLog instance
    // @return a pointer of the SystemLog instance 
    static SystemLog* get_instance();

    // Set SystemLog file 
    // @param file fd of system log file
    void set_log_file(string &file_path);
    
    // print the message in console and save the message
    // @param msg message 
    void log(const string& msg);
    // @param format format string,like printf
    void log(const char* format,...);
    
    // Using this method to write the whole system log
    // to the system log file on disk.
    void persist();
private:
    SystemLog();
    enum SysLogType {_INFO, _WARNING, _ERROR, _DEBUG};
    const char* TypeString[4] = {
        "INFO:",
        "WARNING:",
        "ERROR:",
        "DEBUG:"
    };
    const char* ColorString[4] = {
        "\033[1mINFO:\033[0m",
        "\033[33;1mWARNING:\033[0m",
        "\033[31;1mERROR:\033[0m",
        "\033[34;1mDEBUG:\033[0m"
    };
    std::ofstream sys_log_file;  // fd of system log file
    std::list< std::pair<string, string> > log_list;
    void _log(const char* msg, const SysLogType &type);
    void _log(const char* format, va_list param1, const SysLogType &type);
    void clear();
};

// easy ways to log
void INFOLOG(const string& msg);
void INFOLOG(const char* format,...);
void DEBUGLOG(const string& msg);
void DEBUGLOG(const char* format,...);
void WARNINGLOG(const string& msg);
void WARNINGLOG(const char* format,...);
void ERRORLOG(const string& msg);
void ERRORLOG(const char* format,...);

} // namespace kv_base


#endif // SYSTEMLOG_H