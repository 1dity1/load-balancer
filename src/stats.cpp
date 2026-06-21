#include "../include/stats.h"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

StatsServer::StatsServer(LoadBalancer& lb, int port)
    : lb(lb), port(port), running(false) {}

std::string StatsServer::buildJSON() {
    auto& servers = lb.getServers();
    std::ostringstream json;
    json << "{";
    json << "\"algorithm\":\"Consistent Hashing\",";
    json << "\"servers\":[";
    
    for (size_t i = 0; i < servers.size(); i++) {
        auto& s = servers[i];
        json << "{";
        json << "\"host\":\"" << s.host << "\",";
        json << "\"port\":" << s.port << ",";
        json << "\"alive\":" << (s.alive ? "true" : "false") << ",";
        json << "\"connections\":" << s.active_connections.load();
        json << "}";
        if (i + 1 < servers.size()) json << ",";
    }
    
    json << "]}";
    return json.str();
}

void StatsServer::serve() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(server_sock, (sockaddr*)&addr, sizeof(addr));
    listen(server_sock, 10);

    std::cout << "[StatsServer] Running on port " << port << std::endl;

    while (running) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) continue;

        char buffer[1024];
        recv(client_sock, buffer, sizeof(buffer), 0);

        std::string body = buildJSON();
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: application/json\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "Content-Length: " << body.size() << "\r\n";
        response << "\r\n";
        response << body;

        std::string res_str = response.str();
        send(client_sock, res_str.c_str(), res_str.size(), 0);
        close(client_sock);
    }

    close(server_sock);
}

void StatsServer::start() {
    running = true;
    server_thread = std::thread(&StatsServer::serve, this);
}

void StatsServer::stop() {
    running = false;
    if (server_thread.joinable())
        server_thread.join();
}
