#include "Configuration.h"

namespace kv_engine {

int Configuration::MAX_MEMTABLE_SIZE = 0;

int Configuration::COMPACT_SIZE = 5;

string Configuration::DBLOG_NAME = "DBLOG";

string Configuration::SSTABLE_NAME = "SSTABLE";

Meta * Configuration::meta = nullptr;

string Configuration::DATA_DIR = ".";

string Configuration::LOG_DIR = "./log";

std::map<long, TableReader*> Configuration::TableReaderMap;
}