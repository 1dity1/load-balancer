#include "server.h"

Server::Server(const std::string& h, int p)
    : host(h), port(p), alive(true), active_connections(0) {}

Server::Server(const Server& other)
    : host(other.host), port(other.port),
      alive(other.alive.load()), active_connections(other.active_connections.load()) {}

Server& Server::operator=(const Server& other) {
    if (this != &other) {
        host = other.host;
        port = other.port;
        alive.store(other.alive.load());
        active_connections.store(other.active_connections.load());
    }
    return *this;
}
