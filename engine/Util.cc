#include "Util.h"

#include <stdio.h>

#include <string>

#include <time.h>

#include <string.h>

#include <sstream>

#include <vector>

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

Status SplitFileName(const std::string file_name, std::string & file_head, int & level, long & id) {
    std::vector<std::string> split_string = StringSplit(file_name, "_");
    if (split_string.size() != 3)
        return UnknownError;
    file_head = split_string[0];
    std::stringstream level_stream, id_stream;
    level_stream << split_string[1];
    id_stream << split_string[2];
    
    level_stream >> level;
    id_stream >> id;

    return Success;
}

std::vector<std::string> StringSplit(const std::string & from,const std::string & pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    std::string str = from;

    str += pattern;//扩展字符串以方便操作
    int size=str.size();
    
    for(int i=0; i<size; i++)
    {
        pos=str.find(pattern,i);
        if(pos<size)
        {
        std::string s=str.substr(i,pos-i);
        result.push_back(s);
        i=pos+pattern.size()-1;
        }
    }
    return result;
}