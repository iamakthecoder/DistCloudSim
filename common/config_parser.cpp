#include "config_parser.h"
#include <yaml-cpp/yaml.h>
#include <iostream>

std::vector<NodeInfo> getComputeNodes(const std::string& path) {
    try{
        YAML::Node config = YAML::LoadFile(path);
        std::vector<NodeInfo> nodes;
    
        for (const auto& node : config["cluster"]["compute_nodes"]) {
            nodes.push_back({ node["id"].as<std::string>(), node["ip"].as<std::string>(), node["port"].as<int>() });
        }
        return nodes;
    }
    catch (const std::exception& e) {
        std::cerr << "[ConfigParser] Error loading YAML file to get Compute nodes: " << e.what() << std::endl;
        return {};
    }
}

std::vector<NodeInfo> getControllerNodes(const std::string& path) {
    try{
        YAML::Node config = YAML::LoadFile(path);
        std::vector<NodeInfo> nodes;
    
        for (const auto& node : config["cluster"]["controller_nodes"]) {
            nodes.push_back({ node["id"].as<std::string>(), node["ip"].as<std::string>(), node["port"].as<int>() });
        }
        return nodes;
    }
    catch (const std::exception& e) {
        std::cerr << "[ConfigParser] Error loading YAML file to get Controller nodes: " << e.what() << std::endl;
        return {};
    }
}