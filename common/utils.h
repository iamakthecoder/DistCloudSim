#pragma once
#include <string>

int connect_to(const std::string& ip, int port);
void send_message(int sockfd, const std::string& msg);