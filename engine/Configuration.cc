#include "Configuration.h"

namespace kv_engine {

size_t Configuration::MAX_MEMTABLE_SIZE = 512;

int Configuration::COMPACT_SIZE = 2;

string Configuration::DBLOG_NAME = "DBLOG";

string Configuration::SSTABLE_NAME = "SSTABLE";

Meta * Configuration::meta = nullptr;

string Configuration::DATA_DIR = "./data";

string Configuration::LOG_DIR = "./log";

std::map<long, TableReader*> Configuration::TableReaderMap;
}