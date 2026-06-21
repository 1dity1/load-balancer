#include "loadbalancer.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/select.h>
#include <thread>
#include <chrono>
#include <cerrno>

LoadBalancer::LoadBalancer(int port, std::vector<Server> backends)
    : listen_port_(port), backends_(std::move(backends)),
      current_backend_(0), running_(false), listen_fd_(-1) {}

LoadBalancer::~LoadBalancer() { stop(); }

void LoadBalancer::stop() {
    running_ = false;
    if (listen_fd_ >= 0) {
        close(listen_fd_);
        listen_fd_ = -1;
    }
}

Server& LoadBalancer::getNextBackend() {
    size_t n = backends_.size();
    for (size_t i = 0; i < n; i++) {
        size_t idx = current_backend_.fetch_add(1) % n;
        if (backends_[idx].alive.load())
            return backends_[idx];
    }
    return backends_[current_backend_.fetch_add(1) % n];
}

bool LoadBalancer::connectToBackend(const Server& server, int& backend_fd) {
    backend_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (backend_fd < 0) return false;

    struct hostent* he = gethostbyname(server.host.c_str());
    if (!he) { close(backend_fd); return false; }

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(server.port));
    std::memcpy(&addr.sin_addr, he->h_addr_list[0], static_cast<size_t>(he->h_length));

    if (connect(backend_fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(backend_fd);
        return false;
    }
    return true;
}

void LoadBalancer::forwardTraffic(int client_fd, int backend_fd) {
    char buf[8192];
    int max_fd = std::max(client_fd, backend_fd) + 1;

    while (running_) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(client_fd, &fds);
        FD_SET(backend_fd, &fds);

        struct timeval tv{1, 0};
        int ret = select(max_fd, &fds, nullptr, nullptr, &tv);
        if (ret < 0) break;
        if (ret == 0) continue;

        if (FD_ISSET(client_fd, &fds)) {
            ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
            if (n <= 0) break;
            if (send(backend_fd, buf, static_cast<size_t>(n), 0) <= 0) break;
        }
        if (FD_ISSET(backend_fd, &fds)) {
            ssize_t n = recv(backend_fd, buf, sizeof(buf), 0);
            if (n <= 0) break;
            if (send(client_fd, buf, static_cast<size_t>(n), 0) <= 0) break;
        }
    }

    close(client_fd);
    close(backend_fd);
}

void LoadBalancer::handleClient(int client_fd) {
    Server& backend = getNextBackend();
    std::cout << "Forwarding to " << backend.host << ":" << backend.port << "\n";

    int backend_fd;
    if (!connectToBackend(backend, backend_fd)) {
        std::cerr << "Failed to connect to " << backend.host << ":" << backend.port << "\n";
        backend.alive = false;
        close(client_fd);
        return;
    }

    backend.active_connections++;
    forwardTraffic(client_fd, backend_fd);
    backend.active_connections--;
}

void LoadBalancer::healthCheck() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        for (auto& server : backends_) {
            int fd;
            bool ok = connectToBackend(server, fd);
            if (ok) {
                close(fd);
                if (!server.alive.load())
                    std::cout << "Backend " << server.host << ":" << server.port << " back online\n";
                server.alive = true;
            } else {
                if (server.alive.load())
                    std::cerr << "Backend " << server.host << ":" << server.port << " is down\n";
                server.alive = false;
            }
        }
    }
}

void LoadBalancer::run() {
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) { std::cerr << "socket: " << strerror(errno) << "\n"; return; }

    int opt = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<uint16_t>(listen_port_));

    if (bind(listen_fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "bind: " << strerror(errno) << "\n"; return;
    }
    if (listen(listen_fd_, 128) < 0) {
        std::cerr << "listen: " << strerror(errno) << "\n"; return;
    }

    running_ = true;
    std::thread(&LoadBalancer::healthCheck, this).detach();
    std::cout << "Load balancer running on port " << listen_port_ << "\n";

    while (running_) {
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(listen_fd_, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);
        if (client_fd < 0) {
            if (running_) std::cerr << "accept: " << strerror(errno) << "\n";
            break;
        }
        std::thread(&LoadBalancer::handleClient, this, client_fd).detach();
    }
}
