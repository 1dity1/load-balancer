#include "../include/consistenthash.h"
#include <iostream>

ConsistentHash::ConsistentHash(int virtual_nodes) 
    : virtual_nodes(virtual_nodes) {}

size_t ConsistentHash::hash(const std::string& key) {
    return hasher(key);
}

void ConsistentHash::addServer(Server* server) {
    for (int i = 0; i < virtual_nodes; i++) {
        // Har server ke multiple virtual positions banao
        std::string virtual_key = server->host + ":" + 
                                  std::to_string(server->port) + 
                                  "#" + std::to_string(i);
        size_t position = hash(virtual_key);
        ring[position] = server;
        std::cout << "[Ring] Added " << virtual_key 
                  << " at position " << position << std::endl;
    }
}

void ConsistentHash::removeServer(const std::string& server_key) {
    for (int i = 0; i < virtual_nodes; i++) {
        std::string virtual_key = server_key + "#" + std::to_string(i);
        size_t position = hash(virtual_key);
        ring.erase(position);
        std::cout << "[Ring] Removed " << virtual_key << std::endl;
    }
}

Server* ConsistentHash::getServer(const std::string& client_ip) {
    if (ring.empty()) return nullptr;

    size_t position = hash(client_ip);

    // Clockwise next server dhundo
    auto it = ring.lower_bound(position);

    // Ring wrap around — agar end pe pohonche toh beginning pe jao
    if (it == ring.end()) {
        it = ring.begin();
    }

    return it->second;
}

void ConsistentHash::printRing() {
    std::cout << "\n=== Consistent Hash Ring ===" << std::endl;
    for (auto& [position, server] : ring) {
        std::cout << "Position " << position % 1000 
                  << " → " << server->host 
                  << ":" << server->port << std::endl;
    }
    std::cout << "============================\n" << std::endl;
}