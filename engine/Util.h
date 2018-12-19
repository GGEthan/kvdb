#ifndef _ENGINE_UTIL_H_
#define _ENGINE_UTIL_H_

enum Status {
  Success,
  KeyNotFound,
  KeyExists,
  KeyRemove,
  TableFull,
  IOError,
  UnknownError,
  FileNotFound
};

#define ASSERT(S)                                                              \
  do {                                                                         \
    Status res = S;                                                            \
    if (res != Status::Success)                                                \
      return res;                                                              \
  } while (0)

#define GuardWithMutex(x) std::mutex x_mtx;

#define GuardWithCV(x) std::condition_variable x_cv;

// for system log
enum LogMode {
	Info,
	Warn,
	Error,
	Debug
};

void SysLog(const LogMode mode, const char* format, ...);

void DEBUGLOG(const char* format, ...);
void INFOLOG(const char* format, ...);
void ERRORLOG(const char* format, ...);
void WARNLOG(const char* format, ...);

#include <string>
#include <vector>
std::string ConcatFileName(std::string dir, std::string file_head, int level, long id);

Status SplitFileName(const std::string file_name, std::string & file_hear, int & level, long & id);

std::vector<std::string> StringSplit(const std::string & from,const std::string & pattern);
#endif // _ENGINE_UTIL_H_