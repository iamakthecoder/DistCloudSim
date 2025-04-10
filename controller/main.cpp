// controller/main.cpp
#include <iostream>
#include <thread>
#include <unordered_map>
#include "../common/rpc.h"
#include "../common/config_parser.h"
#include <unistd.h>
#include "image_service.h"
#include <mutex>

using namespace std;

ImageService imageService;

const int CONTROLLER_PORT = 5000; // can be read from config later

std::unordered_map<std::string, std::string> vm_table; // vm_id -> status
std::unordered_map<std::string, std::pair<std::string, int>> vm_to_node_map; // vm_id -> (compute_ip, compute_port)
std::unordered_map<std::string, ImageMetadata> image_table; // vm_id -> image_metadata
std::vector<NodeInfo> compute_nodes; // List of compute nodes
std::mutex compute_mutex;

void handle_client(int client_sock) {
    RPCRequest req = receive_request(client_sock);
    RPCResponse res;

    if (req.command == "create_vm") {
        std::string image_name = req.payload;

        ImageMetadata image;
        if(!imageService.get_image(image_name, image)){
            res.success = false;
            res.data = "Image not found";
            send_response(client_sock, res);
            close(client_sock);
            return;
        }

        // round robin selection of compute nodes as of now
        int id = 0;
        NodeInfo compute = compute_nodes[id];
        id = (id + 1) % compute_nodes.size();

        // Forward to compute node
        RPCRequest forward_req;
        forward_req.command = "create_vm";
        forward_req.payload = image_name; 
        forward_req.payload += (" os:" + image.os);
        forward_req.payload += (" cpu:" + std::to_string(image.cpu));
        forward_req.payload += (" memory:" + std::to_string(image.memory));
        forward_req.payload += " preinstalled:";
        for (const auto& item : image.preinstalled) {
            forward_req.payload += (" " + item);
        }

        RPCResponse compute_res = send_request(compute.ip, compute.port, forward_req);

        if (compute_res.success) {
            std::string vm_id = compute_res.data;
            vm_table[vm_id] = "running";
            vm_to_node_map[vm_id] = {compute.ip, compute.port};
            image_table[vm_id] = image;
            res.success = true;
            res.data = vm_id;
        } else {
            res.success = false;
            res.data = "Failed to create VM";
        }
    }
    else if (req.command == "list_vms") {
        std::string result;
        for (auto& pair : vm_table) {
            result += pair.first + ": " + pair.second;
            auto it = image_table.find(pair.first);
            if(it != image_table.end()) {
                result += " (Image: " + it->second.name + ")\n";
            }
            else{
                result += "\n";
            }
        }
        res.success = true;
        res.data = result;
    }
    else if (req.command == "terminate_vm") {
        std::string vm_id = req.payload;
    
        if (vm_to_node_map.find(vm_id) == vm_to_node_map.end()) {
            res.success = false;
            res.data = "VM ID not found";
        }
        else{
            auto [compute_ip, compute_port] = vm_to_node_map[vm_id];
        
            RPCRequest forward_req;
            forward_req.command = "terminate_vm";
            forward_req.payload = vm_id;
        
            RPCResponse forward_res = send_request(compute_ip, compute_port, forward_req);
            if (forward_res.success) {
                vm_to_node_map.erase(vm_id);
                image_table.erase(vm_id);
                vm_table[vm_id] = "terminated";
                res.success = true;
                res.data = "VM terminated";
            } else {
                res.success = false;
                res.data = "Failed to terminate VM on compute node";
            }
        }
    
    }
    else if (req.command == "list_images"){
        std::vector<std::string> images = imageService.list_image_names();
        std::string response = "";
        for (const auto& name : images) {
            response += name + "\n";
        }
        res.success = true;
        res.data = response;
    }
    else if (req.command == "register_compute"){
        NodeInfo compute_node;
        vector<NodeInfo> config_compute_nodes = getComputeNodes("config/cluster_config.yaml");
        bool found = false;
        for (const auto& node : config_compute_nodes) {
            if (node.id == req.payload) {
                compute_node = node;
                found = true;
                break;
            }
        }
        if(!found){
            res.success = false;
            res.data = "Compute node with given ID not found in the config";
        }
        else{
            std::lock_guard<std::mutex> lock(compute_mutex);
            compute_nodes.push_back(compute_node);
            res.success = true;
            res.data = "Compute node registered";
            cout<<"[Controller] Registered compute node: " << compute_node.id << "\n";
        }
    }
    else {
        res.success = false;
        res.data = "Unknown command";
    }

    send_response(client_sock, res);
    close(client_sock);
}

int main() {
    if (!imageService.load_images_from_disk("images")) {
        std::cerr << "[Controller] Failed to load any images. Exiting.\n";
        return 1;
    }

    int server_sock = start_server(CONTROLLER_PORT);
    std::cout << "[Controller] Listening on port " << CONTROLLER_PORT << "...\n";

    while (true) {
        int client_sock = accept_connection(server_sock);
        std::thread t(handle_client, client_sock);
        t.detach();
    }
    return 0;
}
