#pragma once
#include "loadbalancer.h"
#include <thread>
#include <atomic>
#include <string>

class StatsServer {
public:
    StatsServer(LoadBalancer& lb, int port = 9090);
    void start();
    void stop();

private:
    LoadBalancer& lb;
    int port;
    std::atomic<bool> running;
    std::thread server_thread;
    void serve();
    std::string buildJSON();
};
