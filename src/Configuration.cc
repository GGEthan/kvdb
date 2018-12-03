// Configuration.cpp

#include <utility>

#include "Configuration.h"

#include "DataBase.h"
namespace kv_base {

Configuration* Configuration::get_instance() {
    return Configuration::get_instance(DEFAULT_CONF_PATH);
}

Configuration* Configuration::get_instance(const char* conf_path) {
    static Configuration* conf = NULL;
    if (conf == NULL)
        conf = new Configuration(conf_path);
    return conf;
} 

Configuration::Configuration(const char* conf_path) {
    this->conf_path = conf_path;
    this->sys_log = SystemLog::get_instance();  
}

void Configuration::init() {
    this->init_conf();
    this->init_sys_log();
    this->init_log();
    this->init_data();

}

void Configuration::init_conf() {
    std::ifstream conf_file;
    conf_file.open(this->conf_path.c_str());
    if (!conf_file.is_open()) {
        // using default configuration parameters
        ERRORLOG("Can't open file %s.", this->conf_path.c_str());
    }
    else {
        // using provided configuration file
        INFOLOG("Using configuration file %s", this->conf_path.c_str());
        this->user_conf(conf_file);
    }
    
    // check if there is any required config missing
    this->default_conf();
    // bind strings with the map
    this->bind_conf_path();
}

void Configuration::init_sys_log() {
    INFOLOG("Setting SystemLom file %s", this->sys_log_path.c_str());
    this->sys_log->set_log_file(this->sys_log_path);
    INFOLOG("Initialize SystemLog successfully.");
}

void Configuration::init_data() {
    INFOLOG("Setting DataBase file %s", this->data_path.c_str());
    DataBase* db = DataBase::get_instance();
    if (!db->init(this->data_path)) {
        ERRORLOG("Fail to initialize the DataBase. ");
        EXIT();
    }
    INFOLOG("Initialize DataBase successfully.");
    
    // Recover DataBase
    INFOLOG("Recovering DataBase from DataLog.");
    if (!db->recover()) 
        ERRORLOG("Fail to recover the DataBase");
    INFOLOG("Recover DataBase successfully");
}

void Configuration::init_log() {
    INFOLOG("Setting DataLog file %s", this->log_path.c_str());
    DataLog* dl = DataLog::get_instance();
    if (!dl->init(this->log_path)) {
        ERRORLOG("Fail to initialize the DataLog.");
        EXIT();
    }
    INFOLOG("Initialize DataLog successfully.");
}

void Configuration::user_conf(std::ifstream &conf_file) {
    if (!conf_file.is_open()) 
        // using the function in the wrong way
        DEBUGLOG("Can't open configuration file.");
    else {
        // read configuration file
        char buf[1024];
        while (conf_file.getline(buf,1024)) {
            string line(buf);
            // DEBUGLOG("Read configuration line: %s",line.c_str());
            // erase ' '
            int blank = -1;
            while ((blank = line.find(' ')) != -1) {
                line = line.erase(blank,1);
            }
            // comment line
            if (line[0] == '#')
                continue;
            // split key and value
            int mid = -1;
            if ((mid = line.find_first_of('=')) != -1) {
                string key = line.substr(0,mid),
                       value = line.substr(mid + 1);
                this->conf_kv.insert(std::pair<string,string>(key,value));
                DEBUGLOG("Add configutarion key: %s , value : %s",key.c_str(),value.c_str());
            }
        }
    }
}

void Configuration::default_conf() {
    using std::pair;
    if (conf_kv.find("DATA_PATH") == conf_kv.end()) {
        WARNINGLOG("Using default DATA_PATH: %s", DEFAULT_DATA_PATH);
        conf_kv.insert(pair<string, string>("DATA_PATH", DEFAULT_DATA_PATH));
    }
    if (conf_kv.find("SYS_LOG_PATH") == conf_kv.end()) {
       WARNINGLOG("Using default SYS_LOG_PATH: %s", DEFAULT_SYS_LOG_PATH);
        conf_kv.insert(pair<string, string>("SYS_LOG_PATH", DEFAULT_SYS_LOG_PATH));
    }
    if (conf_kv.find("LOG_PATH") == conf_kv.end()) {
        WARNINGLOG("Using default LOG_PATH: %s", DEFAULT_LOG_PATH);
        conf_kv.insert(pair<string, string>("LOG_PATH", DEFAULT_LOG_PATH));
    }
}

void Configuration::bind_conf_path() {
    this->sys_log_path = this->conf_kv["SYS_LOG_PATH"];
    this->data_path = this->conf_kv["DATA_PATH"];
    this->log_path = this->conf_kv["LOG_PATH"];
}

void EXIT() {
    WARNINGLOG("System exiting...");
    WARNINGLOG("Saving System Log in file %s", Configuration::get_instance()->sys_log_path.c_str());
    // save System Log
    SystemLog *sys_log = SystemLog::get_instance();
    sys_log->persist();
    // save Data
    // DataBase *db = DataBase::get_instance();
    // db->persist();
    // save Log
    DataLog *dl = DataLog::get_instance();
    dl->persist();

    exit(1);
}

} // namespace kv_base