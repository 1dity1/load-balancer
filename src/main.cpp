#include "../include/loadbalancer.h"
#include "../include/healthchecker.h"
#include "../include/stats.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>

#define PORT 8080
#define BUFFER_SIZE 4096

extern void forwardRequest(Server* server, const char* request, int req_len);

std::string getClientIP(int client_sock) {
    sockaddr_in client_addr{};
    socklen_t len = sizeof(client_addr);
    getpeername(client_sock, (sockaddr*)&client_addr, &len);
    return std::string(inet_ntoa(client_addr.sin_addr));
}

void handleClient(int client_sock, LoadBalancer& lb) {
    char buffer[BUFFER_SIZE];
    int bytes = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes <= 0) {
        close(client_sock);
        return;
    }
    buffer[bytes] = '\0';

    std::string client_ip = getClientIP(client_sock);
    std::cout << "[Client] " << client_ip << std::endl;

    Server* server = lb.getNextServer(client_ip);
    if (!server) {
        std::cerr << "[-] No available servers!" << std::endl;
        const char* err = "HTTP/1.1 503 Service Unavailable\r\n\r\n";
        send(client_sock, err, strlen(err), 0);
        close(client_sock);
        return;
    }

    std::cout << "[->] Forwarding " << client_ip
              << " to " << server->host
              << ":" << server->port << std::endl;

    forwardRequest(server, buffer, bytes);
    close(client_sock);
}

int main() {
    LoadBalancer lb(Algorithm::CONSISTENT_HASHING);

    lb.addServer("127.0.0.1", 8081);
    lb.addServer("127.0.0.1", 8082);
    lb.addServer("127.0.0.1", 8083);

    lb.printServers();

    HealthChecker hc(lb, 5);
    hc.start();

    StatsServer stats(lb, 9090);
    stats.start();

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_sock, (sockaddr*)&addr, sizeof(addr));
    listen(server_sock, 10);

    std::cout << "[*] Load Balancer running on port " << PORT << std::endl;
    std::cout << "[*] Algorithm: Consistent Hashing with Virtual Nodes" << std::endl;
    std::cout << "[*] Health Checker running every 5s" << std::endl;
    std::cout << "[*] Stats API running on port 9090\n" << std::endl;

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (sockaddr*)&client_addr, &client_len);

        if (client_sock < 0) continue;

        std::thread(handleClient, client_sock, std::ref(lb)).detach();
    }

    close(server_sock);
    return 0;
}