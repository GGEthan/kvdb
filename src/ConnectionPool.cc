// // ConnectionPool.cpp

// #include <sys/socket.h>

// #include "ConnectionPool.h"

// #include "SystemLog.h"

// #include "Configuration.h"
// namespace kv_base {

// ConnectionPool* ConnectionPool::get_instance(int max_connection) {
//     static ConnectionPool* pool = NULL;
//     if (pool == NULL)
//         pool = new ConnectionPool(max_connection);
//     return pool;
// }

// ConnectionPool::ConnectionPool(int max_connection) {
//     SystemLog* sys_log = SystemLog::get_instance();

//     if (max_connection <= 0) {
//         sys_log->log("illegal max_connection: %d",max_connection);
//         EXIT();
//     }
//     sys_log->log("Initializing work threads.");
    
//     free = max_connection;

//     // initialize work threads
//     for(int i = 0; i < max_connection; i++)
//         thread_pool.push_back(std::thread(
//             run_thread,
//             i,
//             &(this->free),
//             &(this->mutex),
//             &(this->new_socket)
//         ));
//     sys_log->log("Create %d working threads.",max_connection);

// }

// void ConnectionPool::connect(int sockfd) {
//     SystemLog* sys_log = SystemLog::get_instance();
//     std::lock_guard<std::mutex> locker(this->mutex);
//     if (this->free == 0) {
//         // full pool
//         sys_log->log("Get a new connection, but the ConnectionPool is full.");
//         shutdown(sockfd,SHUT_RDWR);
//     }
//     this->new_socket.enqueue(sockfd);
// }

// void run_thread(int id, int* free, std::mutex* mtx, SafeQueue<int>* socket_list) {
//     int sock_fd;
//     SystemLog* sys_log = SystemLog::get_instance();

//     while(1) {
//         socket_list->dequeue_blocking(sock_fd);
//         sys_log->log("New connection %d in thread %d.",sock_fd,id);
//         mtx->lock();
//         (*free)--;
//         mtx->unlock();
        
//         // deal with the socket
//         // ...

//         sys_log->log("Shutdown connection %d in thread %d.",sock_fd,id);
//         mtx->lock();
//         (*free)++;
//         mtx->unlock();
//     } 
// }


// } // namespace kv_base