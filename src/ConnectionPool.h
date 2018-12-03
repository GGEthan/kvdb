// ConnectionPool.h

#ifndef CONNECTIONPOLL_H
#define CONNECTIONPOLL_H

#include <mutex>

#include <list>

#include <thread>

#include "common.h"

#include "SafeQueue.hpp"

namespace kv_base {

// ConnectionPool is a singleton class. 
class ConnectionPool {
public: 
    static ConnectionPool* get_instance(int max_connection);
    // Get a new connection and send it
    // to a working thread
    // @param sockfd socket fd of the new connection
    void connect(int sockfd);
private:
    ConnectionPool(int max_connection);
    std::mutex mutex; // for free 
    int free;
    std::list<std::thread> thread_pool;
    SafeQueue<int> new_socket;
};
// The entrance of the working thread
void run_thread(int id, int* free, std::mutex* mtx, SafeQueue<int>* socket_list);
} // namespcae kv_base

#endif // CONNECTIONPOLL_H