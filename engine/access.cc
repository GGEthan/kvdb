#include "access.h"

bool Access::open(const char* log_dir, const char* data_dir) {
	std::string log_dir_s = log_dir;
	std::string data_dir_s = data_dir;
	return !engine_->Open(log_dir_s,data_dir_s);
}

bool Access::write(const char* key, const char* value) {
	std::string key_s = key;
	std::string value_s = value;
	return !engine_->Put(key_s,value_s,0);
}

bool Access::update(const char* key, const char* value) {
	std::string key_s = key;
	std::string value_s = value;
	return !engine_->Put(key_s,value_s,1);
}

const char* Access::read(const char* key) {
	std::string key_s = key;
	kv_engine::ValueType value;
	bool ret = !engine_->Get(key_s,value);
	if(ret)
	{
		return std::string(value.data(),value.size()).c_str();
	}
	else 
		return 0;

}

bool Access::remove(const char* key) {
	std::string key_s = key;
	return !engine_->Delete(key_s);
}