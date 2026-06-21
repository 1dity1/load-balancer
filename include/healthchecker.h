#pragma once
#include "loadbalancer.h"
#include <thread>
#include <atomic>

class HealthChecker {
public:
    HealthChecker(LoadBalancer& lb, int interval_seconds = 5);
    ~HealthChecker();
    void start();
    void stop();

private:
    LoadBalancer& lb;
    int interval;
    std::atomic<bool> running;
    std::thread checker_thread;
    void checkServers();
    bool pingServer(const std::string& ip, int port);
};