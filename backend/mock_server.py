#!/usr/bin/env python3
import socket
import sys
import threading

def handle_client(conn, addr, server_id, port):
    with conn:
        data = conn.recv(4096)
        if data:
            body = f"Response from server {server_id} (port {port})\n"
            response = (
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                f"Content-Length: {len(body)}\r\n"
                "Connection: close\r\n"
                "\r\n"
                + body
            )
            conn.sendall(response.encode())

def run_server(port, server_id):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(("", port))
        s.listen(5)
        print(f"Server {server_id} listening on port {port}", flush=True)
        while True:
            conn, addr = s.accept()
            threading.Thread(target=handle_client, args=(conn, addr, server_id, port), daemon=True).start()

if __name__ == "__main__":
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8081
    server_id = sys.argv[2] if len(sys.argv) > 2 else str(port)
    run_server(port, server_id)
