#pragma once
#include <string>
#include <atomic>

struct Server {
    std::string host;
    int port;
    std::atomic<bool> alive;
    std::atomic<int> active_connections;

    Server(const std::string& host, int port);
    Server(const Server& other);
    Server& operator=(const Server& other);
};
