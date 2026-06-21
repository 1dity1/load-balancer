#include "loadbalancer.h"
#include <iostream>
#include <vector>
#include <csignal>
#include <cstdlib>

static LoadBalancer* g_lb = nullptr;

void signalHandler(int) {
    if (g_lb) g_lb->stop();
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    int listen_port = (argc > 1) ? std::atoi(argv[1]) : 8080;

    std::vector<Server> backends = {
        Server("127.0.0.1", 8081),
        Server("127.0.0.1", 8082),
        Server("127.0.0.1", 8083),
    };

    LoadBalancer lb(listen_port, std::move(backends));
    g_lb = &lb;

    std::cout << "Starting load balancer on port " << listen_port << "\n";
    lb.run();
    return 0;
}
