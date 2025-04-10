#include "rpc.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <bits/stdc++.h>

using namespace std;

const int BUF_SIZE = 4096;

int start_server(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(sock, (sockaddr*)&addr, sizeof(addr));
    listen(sock, 10);
    return sock;
}

int accept_connection(int server_sock) {
    sockaddr_in client;
    socklen_t len = sizeof(client);
    return accept(server_sock, (sockaddr*)&client, &len);
}

RPCRequest receive_request(int client_sock) {
    char buffer[BUF_SIZE];
    std::string raw;
    int bytes;

    int separator_count = 0;
    while (separator_count < 2 && (bytes = read(client_sock, buffer, BUF_SIZE)) > 0) {
        raw.append(buffer, bytes);
        separator_count = count(raw.begin(), raw.end(), '|');
    }

    size_t first_sep = raw.find("|");
    size_t second_sep = raw.find("|", first_sep + 1);

    return { raw.substr(0, first_sep), raw.substr(first_sep + 1, second_sep - first_sep - 1) };
}

void send_response(int client_sock, const RPCResponse& res) {
    std::string msg = (res.success ? "1" : "0") + std::string("|") + res.data + "|";
    send(client_sock, msg.c_str(), msg.length(), 0);
}

RPCResponse send_request(const std::string& ip, int port, const RPCRequest& req) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv {};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serv.sin_addr);

    connect(sock, (sockaddr*)&serv, sizeof(serv));

    std::string msg = req.command + "|" + req.payload + "|";
    send(sock, msg.c_str(), msg.length(), 0); // send the request

    // Read the response
    char buffer[BUF_SIZE];
    std::string raw;
    int bytes;
    int separator_count = 0;
    while (separator_count < 2 && (bytes = read(sock, buffer, BUF_SIZE)) > 0) {
        raw.append(buffer, bytes);
        separator_count = count(raw.begin(), raw.end(), '|');
    }
    close(sock);
    size_t first_sep = raw.find("|");
    size_t second_sep = raw.find("|", first_sep + 1);
    bool success = (raw[0] == '1');
    std::string data = raw.substr(first_sep + 1, second_sep - first_sep - 1);
    return { success, data }; // return the response
}