#include "../include/loadbalancer.h"
#include <iostream>

LoadBalancer::LoadBalancer(Algorithm algo) 
    : current_index(0), algo(algo), ch(3) {}

void LoadBalancer::addServer(const std::string& ip, int port) {
    std::lock_guard<std::mutex> lock(mtx);
    servers.emplace_back(ip, port);
    ch.addServer(&servers.back());
    std::cout << "[+] Server added: " << ip << ":" << port << std::endl;
}

Server* LoadBalancer::roundRobin() {
    if (servers.empty()) return nullptr;

    int total = servers.size();
    for (int i = 0; i < total; i++) {
        int idx = current_index.fetch_add(1) % total;
        if (servers[idx].alive) {
            servers[idx].active_connections++;
            return &servers[idx];
        }
    }
    return nullptr;
}

Server* LoadBalancer::consistentHash(const std::string& client_ip) {
    Server* server = ch.getServer(client_ip);
    if (server && server->alive) {
        server->active_connections++;
        return server;
    }
    std::cout << "[!] CH server down, falling back to Round Robin" << std::endl;
    return roundRobin();
}

Server* LoadBalancer::getNextServer(const std::string& client_ip) {
    std::lock_guard<std::mutex> lock(mtx);
    
    if (algo == Algorithm::CONSISTENT_HASHING && !client_ip.empty()) {
        return consistentHash(client_ip);
    }
    return roundRobin();
}

std::vector<Server>& LoadBalancer::getServers() {
    return servers;
}

void LoadBalancer::printServers() {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "\n=== Server Pool ===" << std::endl;
    for (auto& s : servers) {
        std::cout << s.host << ":" << s.port 
                  << " | alive: " << (s.alive ? "YES" : "NO")
                  << " | connections: " << s.active_connections 
                  << std::endl;
    }
    std::cout << "==================\n" << std::endl;
}