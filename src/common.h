// common.h

// Include frequently-used headers and define 
// global variables or macros here.
// Every header file of the project should 
// include this file.


#ifndef COMMON_H
#define COMMON_H

#include <string>

#include <fstream>

#include <list>

#include <map>

#ifndef DEFAULT_CONF_PATH
#define DEFAULT_CONF_PATH "conf"
#endif // DEFAULT_CONF_PATH

#ifndef DEFAULT_LOG_PATH
#define DEFAULT_LOG_PATH "log"
#endif // DEFAULT_LOG_PATH

#ifndef DEFAULT_DATA_PATH
#define DEFAULT_DATA_PATH "data"
#endif // DEFAULT_DATA_PATH

#ifndef DEFAULT_SYS_LOG_PATH
#define DEFAULT_SYS_LOG_PATH "sys_log"
#endif // DEFAULT_SYS_LOG_PATH


#ifndef DEBUG
#define DEBUG 1
#endif
#endif // COMMON_H