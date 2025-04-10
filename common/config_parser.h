#pragma once
#include <string>
#include <vector>

struct NodeInfo {
    std::string id;
    std::string ip;
    int port;
};

std::vector<NodeInfo> getComputeNodes(const std::string& path);
std::vector<NodeInfo> getControllerNodes(const std::string& path);
