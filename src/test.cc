#include "Configuration.h"

#include "SystemLog.h"

#include "DataBase.h"

using namespace kv_base;

int main(int argc, char *argv[]) {
    
    Configuration* conf;
    if (argc == 1)
        conf = Configuration::get_instance();
    else
        conf = Configuration::get_instance(argv[1]);
    conf->init();

    DataBase* db = DataBase::get_instance();
    return 0;
}