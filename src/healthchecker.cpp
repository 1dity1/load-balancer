#include "../include/healthchecker.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>

HealthChecker::HealthChecker(LoadBalancer& lb, int interval_seconds)
    : lb(lb), interval(interval_seconds), running(false) {}

HealthChecker::~HealthChecker() {
    stop();
}

void HealthChecker::start() {
    running = true;
    checker_thread = std::thread(&HealthChecker::checkServers, this);
    std::cout << "[HealthChecker] Started - checking every "
              << interval << "s" << std::endl;
}

void HealthChecker::stop() {
    running = false;
    if (checker_thread.joinable())
        checker_thread.join();
}

void HealthChecker::checkServers() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(interval));
        auto& servers = lb.getServers();
        for (auto& server : servers) {
            bool alive = pingServer(server.host, server.port);
            if (alive && !server.alive) {
                std::cout << "[HealthChecker] Server BACK ONLINE: "
                          << server.host << ":" << server.port << std::endl;
            } else if (!alive && server.alive) {
                std::cout << "[HealthChecker] Server DOWN: "
                          << server.host << ":" << server.port << std::endl;
            }
            server.alive = alive;
        }
    }
}

bool HealthChecker::pingServer(const std::string& ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    bool reachable = (connect(sock, (sockaddr*)&addr, sizeof(addr)) == 0);
    close(sock);
    return reachable;
}
