// SocketServer.h

#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include "common.h"

#include "SystemLog.h"

namespace kv_base {

// SocketServer is a singleton class.
class SocketServer {
public:

    static SocketServer* get_instance();
    static SocketServer* get_instance(const char* ip, int port);
    // Bind the socket with the port and begin listening
    // @param max_connection max connection for listen()
    // @return 0 if there is any error, otherwise 1.
    int start_and_listen(int max_connection);

    // Accept a connection 
    // @output sockfd socket fd of the connection
    // @return 0 if there is any error, otherwise 1. 
    int accept_connection(int& sockfd);
private:
    SocketServer(const char* ip, int port);
    int server_socket;
    string ip;
    int port;

};
} // namespace kv_base

#endif // SOCKETSERVER