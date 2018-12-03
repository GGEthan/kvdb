// Configuration.h

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <map>

#include "common.h"

#include "SystemLog.h"

using std::string;
namespace kv_base {
 
// Configuration is a singleton class(or object) 
// that can save global configuravle varibales.
// Besides, Configuration provides methods to read  
// configuration file and initialize the system.
// Using get_instance() to get or create the only instance 
// of Configutation.
class Configuration {
public:
    char ip[16];  // server ip string
    int port;  // server port

    string conf_path, 
           data_path, 
           log_path,  
           sys_log_path;
    
    // Create a Configuration instance using default
    // conf file path or get the instance.
    // @return a pointer of the Configuration instance
    static Configuration* get_instance();
    
    // Create the Configuration instance.
    // @param conf_path path of the configutation file
    // @return a pointer of the Configuration instance
    static Configuration* get_instance(const char* conf_path);

    // Initialize the system include reading conf file
    // and restoring data&log files.
    void init();

    // Get the value with key in configuration map
    // @param key key in map
    // @param output parameter value in map
    // @return 0 if there is any error, otherwise 1
    int get_value(string &key,string &value);

    // Using EXIT() to save the System Log file 
    // and exit the system
    friend void EXIT();

private:
    // @param conf_path path of the configutation file
    Configuration(const char* conf_path);
    
    std::map<string,string> conf_kv; // save config kv from config file.

    // configurable parameters

    SystemLog* sys_log;
    void init_conf();
    void init_sys_log();
    void init_data();
    void init_log();
    void default_conf();
    void user_conf(std::ifstream &conf_file);
    void bind_conf_path();
};

// Using EXIT() to save the System Log file and
// exit the system
void EXIT();
} // namespace kv_base


#endif // CONFIGURATION_H