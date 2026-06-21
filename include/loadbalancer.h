#pragma once
#include "server.h"
#include "consistenthash.h"
#include <vector>
#include <mutex>
#include <atomic>

enum class Algorithm {
    ROUND_ROBIN,
    CONSISTENT_HASHING
};

class LoadBalancer {
public:
    LoadBalancer(Algorithm algo = Algorithm::ROUND_ROBIN);
    void addServer(const std::string& ip, int port);
    Server* getNextServer(const std::string& client_ip = "");
    std::vector<Server>& getServers();
    void printServers();

private:
    std::vector<Server> servers;
    std::atomic<int> current_index;
    std::mutex mtx;
    Algorithm algo;
    ConsistentHash ch;

    Server* roundRobin();
    Server* consistentHash(const std::string& client_ip);
};