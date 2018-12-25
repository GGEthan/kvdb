#include "KVEngine.h"

class Access {
public:
	Access(){
		engine_ = new kv_engine::KVEngine();
	}
	~Access();

	bool open(const char* log_dir, const char* data_dir);

	bool write(const char* key, const char* value);

	bool update(const char* key, const char* value);

	const char* read(const char* key);

	bool remove(const char* key);

protected:
	kv_engine::EngineBase* engine_;
};
