#pragma once
#include "server.h"
#include <map>
#include <string>
#include <vector>
#include <functional>

class ConsistentHash {
public:
    ConsistentHash(int virtual_nodes = 3);
    void addServer(Server* server);
    void removeServer(const std::string& server_key);
    Server* getServer(const std::string& client_ip);
    void printRing();

private:
    int virtual_nodes;
    std::map<size_t, Server*> ring; // hash position → server
    std::hash<std::string> hasher;
    
    size_t hash(const std::string& key);
};