# Load Balancer in C++

A production-grade TCP Load Balancer built from scratch in C++ with multiple load balancing algorithms, automatic health checking, and a real-time monitoring dashboard.

## Architecture
Client Requests (HTTP/TCP)

↓

[Load Balancer - C++] :8080

├── Round Robin Algorithm

├── Consistent Hashing + Virtual Nodes

├── Health Checker (every 5s)

└── Stats API :9090

↓

Backend Servers (8081 / 8082 / 8083)

↓

[React Dashboard] :3000 (real-time monitoring)

## Features
- TCP Load Balancing on port 8080
- Round Robin request distribution
- Consistent Hashing with Virtual Nodes for session affinity
- Background Health Checker every 5 seconds
- Automatic failover on server crash
- Auto-recovery detection when server comes back online
- Multi-threaded request handling
- HTTP Stats API in C++ for real-time metrics
- React monitoring dashboard with live server status

## Tech Stack
- C++17, POSIX Sockets, pthreads, CMake
- React.js (monitoring dashboard)
- Python (mock backend servers)

## Run it

Start backends:
python3 backend/mock_server.py 8081 1
python3 backend/mock_server.py 8082 2
python3 backend/mock_server.py 8083 3

Build and run load balancer:
mkdir build && cd build
cmake .. && make
./load_balancer

Start dashboard:
cd dashboard/monitor
npm start

Test:
curl http://localhost:8080

## Phases
- Phase 1: TCP Listener + Round Robin done
- Phase 2: Health Checker + Automatic Failover done
- Phase 3: Consistent Hashing with Virtual Nodes done
- Phase 4: Real-time React Monitoring Dashboard done