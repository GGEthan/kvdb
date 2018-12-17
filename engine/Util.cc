#include "Util.h"

#include <stdio.h>

#include <string>

#include <time.h>

#include <string.h>

#include <sstream>

void SysLog(const LogMode mode, const char * format, va_list args) {
    char log_buf[512];
    sprintf(log_buf, "%d %s %s\n", time(NULL), LogString[mode], format);
    //strcat(log_buf, format);
    vprintf(log_buf, args);
}

void DEBUGLOG(const char * format, ...) {
#ifdef DEBUG    
    va_list args;
    va_start(args, format);
    SysLog(LogMode::Debug, format, args);
    va_end(args);
#endif
}

void ERRORLOG(const char * format, ...) {
    va_list args;
    va_start(args, format);
    SysLog(LogMode::Error, format, args);
    va_end(args);
}

void INFOLOG(const char * format, ...) {
    va_list args;
    va_start(args, format);
    SysLog(LogMode::Info, format, args);
    va_end(args);
}


void WARNLOG(const char * format, ...) {
    va_list args;
    va_start(args, format);
    SysLog(LogMode::Warn, format, args);
    va_end(args);
}

std::string ConcatFileName(std::string dir, std::string file_head, int level, long id) {
    std::stringstream ss;
    ss << dir << "/" << file_head << "_" << level << "_" << id;
    std::string result;
    ss >> result;
    return result;
}