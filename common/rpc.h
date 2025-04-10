#pragma once
#include <string>

struct RPCRequest {
    std::string command;
    std::string payload;
};

struct RPCResponse {
    bool success;
    std::string data;
};

int start_server(int port);
int accept_connection(int server_sock);
RPCRequest receive_request(int client_sock);
void send_response(int client_sock, const RPCResponse& res);
RPCResponse send_request(const std::string& ip, int port, const RPCRequest& req);
