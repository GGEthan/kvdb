#include "KVEngine.h"

#include <map>

#include <random>

#include <sstream>

#include <iostream>

using namespace std;

map<string, string> KVMAP;

string CreateKey(unsigned int n) {
    stringstream ss;
    string result;    
    ss << n;
    ss >> result;
    return result;
}

string CreateValue(unsigned int n) {
    stringstream ss;
    string result;
    for (unsigned int i = 0; i < n % 100; i++) 
        ss << n;
    ss >> result;
    return result;
}

string ValueFromKey(const string & key) {
    stringstream ss;
    ss << key;
    unsigned int n;
    ss >> n;
    return CreateValue(n);
}

void CreateData(int size) {
    default_random_engine e;
    e.seed(0);
    size_t key_size = 0;
    size_t value_size = 0;
    for(int i = 0; i < size; i++) {
        unsigned int r = e();
        string key = CreateKey(r);
        if (KVMAP.find(key) != KVMAP.end()) {
            i--;
            continue;
        }
        string value = CreateValue(r);
        key_size += key.size();
        value_size += value.size();
        KVMAP[key] = value;
    }    
    cout << "KVMAP count:" << KVMAP.size() << endl;
    cout << "key size:" << key_size << " average:" << key_size / size << endl;
    cout << "value size:" << value_size << " average:" << value_size / size << endl;
}


int main() {
    kv_engine::EngineBase* engine = new kv_engine::KVEngine();
    engine->Open("Log", "Data");

    CreateData(20);
    // engine->Put("abc", "abcdefg", true);

    // engine->Put("ccc", "ccccc", true);
    for (auto iter : KVMAP) {
        engine->Put(iter.first, iter.second, true);
        cout << "Put : " << iter.first << endl;
    }

    for (auto iter : KVMAP) {
        kv_engine::ValueType value;
        Status res;
        res = engine->Get(iter.first, value);
        if (res != Success) {
            cout << "Error : " << res << endl;
            continue;
        }
        if (ValueFromKey(iter.first) != string(value.data(), value.size())) {
            cout << "Wrong Value. key:" << iter.first <<" value:" << ValueFromKey(iter.first)
                 << " get:" << string(value.data(), value.size()) << endl;
        } else{
            cout << "OK! " << iter.first << endl;
        }
    }

    delete engine;

    return 0;
}