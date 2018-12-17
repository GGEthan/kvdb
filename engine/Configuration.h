#ifndef _ENGINE_CONFIGURATION_H_
#define _ENGINE_CONFIGURATION_H_

#include <string>

#include <map>

#include "Meta.h"

#include "TableIO.h"

namespace kv_engine {
using std::string;
class Configuration {
public:
	static int MAX_MEMTABLE_SIZE;

	static int COMPACT_SIZE;
	
	static string SSTABLE_NAME;
	
	static string DBLOG_NAME;
	
	static Meta * meta;

	static string DATA_DIR;

	static std::map<long, TableReader*> TableReaderMap;

	static void init() {
		MAX_MEMTABLE_SIZE = 0;
	}
	
};
} // namespace kv_engine

#endif