#include "KVEngine.h"

int main() {
    kv_engine::EngineBase* engine = new kv_engine::KVEngine();
    engine->Open("Log", "Data");

    engine->Put("abc", "abcdefg", true);

    engine->Put("ccc", "ccccc", true);

    delete engine;
    return 0;
}