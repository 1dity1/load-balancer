# Load Balancer in C++

A production-grade TCP Load Balancer built from scratch in C++ with Round Robin scheduling and automatic Health Checking.

## Architecture
Client Requests (HTTP/TCP)

↓

[Load Balancer - C++] :8080

├── Round Robin Algorithm

├── Health Checker (every 5s)

└── Multi-threaded Request Handling

↓

Backend Servers (9001 / 9002 / 9003)
## Features
- TCP Load Balancing on port 8080
- Round Robin request distribution
- Background Health Checker every 5 seconds
- Automatic failover on server crash
- Auto-recovery detection
- Multi-threaded request handling

## Tech Stack
- C++17, POSIX Sockets, pthreads, CMake

## Run it
Start backends: python3 backend/mock_server.py 8081 1
Build: mkdir build && cd build && cmake .. && make
Run: ./load_balancer
Test: curl http://localhost:8080

## Phases
- Phase 1: TCP Listener + Round Robin done
- Phase 2: Health Checker + Failover done
- Phase 3: Consistent Hashing coming soon
- Phase 4: Monitoring Dashboard coming soon
