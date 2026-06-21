#pragma once
#include "server.h"
#include <vector>
#include <atomic>
#include <thread>

class LoadBalancer {
public:
    LoadBalancer(int port, std::vector<Server> backends);
    ~LoadBalancer();
    void run();
    void stop();

private:
    int listen_port_;
    std::vector<Server> backends_;
    std::atomic<size_t> current_backend_;
    std::atomic<bool> running_;
    int listen_fd_;

    Server& getNextBackend();
    void handleClient(int client_fd);
    void forwardTraffic(int client_fd, int backend_fd);
    bool connectToBackend(const Server& server, int& backend_fd);
    void healthCheck();
};
