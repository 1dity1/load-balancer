#include "../include/server.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define BUFFER_SIZE 4096

Server::Server(const std::string& h, int p)
    : host(h), port(p), alive(true), active_connections(0) {}

Server::Server(const Server& other)
    : host(other.host), port(other.port),
      alive(other.alive.load()),
      active_connections(other.active_connections.load()) {}

Server& Server::operator=(const Server& other) {
    if (this != &other) {
        host = other.host;
        port = other.port;
        alive.store(other.alive.load());
        active_connections.store(other.active_connections.load());
    }
    return *this;
}

void forwardRequest(Server* server, const char* request, int req_len) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server->port);
    inet_pton(AF_INET, server->host.c_str(), &server_addr.sin_addr);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        server->alive = false;
        close(sock);
        return;
    }

    send(sock, request, req_len, 0);

    char response[BUFFER_SIZE];
    int bytes = recv(sock, response, BUFFER_SIZE - 1, 0);
    if (bytes > 0) {
        response[bytes] = '\0';
        std::cout << "[Response from " << server->host
                  << ":" << server->port << "]\n"
                  << response << std::endl;
    }

    server->active_connections--;
    close(sock);
}
