// compute/main.cpp
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unordered_map>
#include <csignal>
#include <thread>
#include <mutex>
#include <signal.h>

#include "../common/rpc.h"
#include "../common/config_parser.h"
#include "../common/utils.h"

using namespace std;

std::unordered_map<std::string, pid_t> vm_process_table;
std::unordered_map<std::string, std::string> image_table; // vm_id -> image_metadata
std::mutex vm_mutex;
int vm_counter = 0;

void simulate_vm_process() {
    sleep(30); // Simulated workload
    // exit(0);
    while(true){};
}

std::string spawn_vm(std::string image) {
    pid_t pid = fork();
    if (pid == 0) {
        simulate_vm_process();
    } else if (pid > 0) {
        std::lock_guard<std::mutex> lock(vm_mutex);
        std::string vm_id = "vm_" + std::to_string(++vm_counter);
        vm_process_table[vm_id] = pid;
        image_table[vm_id] = image; // Store image metadata
        std::cout << "[Compute] Spawned VM with ID: " << vm_id << ", PID: " << pid << ", Image-metadata: " << image << "\n";
        return vm_id;
    } else {
        return ""; // Failed to fork
    }
    return "";
}

void handle_controller(int sock) {
    RPCRequest req = receive_request(sock);
    RPCResponse res;

    if (req.command == "create_vm") {
        std::string vm_id = spawn_vm(req.payload);
        if (!vm_id.empty()) {
            res.success = true;
            res.data = vm_id;
        } else {
            res.success = false;
            res.data = "VM creation failed";
        }
    } 
    else if (req.command == "terminate_vm") {
        std::string vm_id = req.payload;
    
        if (vm_process_table.find(vm_id) == vm_process_table.end()) {
            res.success = false;
            res.data = "VM ID not found";
        }
        else{
            pid_t pid = vm_process_table[vm_id];
            if (kill(pid, SIGTERM) == 0) {
                vm_process_table.erase(vm_id);
                image_table.erase(vm_id);
                res.success = true;
                res.data = "VM terminated";
            } else {
                res.success = false;
                res.data = "Failed to terminate VM";
            }
        }
    }
    else {
        res.success = false;
        res.data = "Unknown command";
    }

    send_response(sock, res);
    close(sock);
}

bool registerWithController(NodeInfo selfInfo, NodeInfo controllerInfo) {
    RPCRequest req;
    req.command = "register_compute";
    req.payload = selfInfo.id;
    
    int sock = connect_to(controllerInfo.ip, controllerInfo.port);
    RPCResponse res = send_request(controllerInfo.ip, controllerInfo.port, req);
    close(sock);

    if (res.success) {
        std::cout << "[Compute] Registered with controller: " << controllerInfo.id << "\n";
        return true;
    } else {
        std::cerr << "[Compute] Failed to register with controller: " << controllerInfo.id << "\n";
        return false;
    }
}

void reap_zombie_processes() {
    while (true) {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid > 0) {
            std::lock_guard<std::mutex> lock(vm_mutex);
            for (auto it = vm_process_table.begin(); it != vm_process_table.end(); ++it) {
                if (it->second == pid) {
                    std::cout << "[Compute] VM terminated: " << it->first << "\n";
                    vm_process_table.erase(it);
                    break;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "[Compute] Usage: " << argv[0] << " <compute_id>\n";
        return 1;
    }

    NodeInfo selfInfo;
    vector<NodeInfo> compute_nodes = getComputeNodes("config/cluster_config.yaml");
    bool found = false;
    for (const auto& node : compute_nodes) {
        if (node.id == argv[1]) {
            selfInfo = node;
            found = true;
            break;
        }
    }
    if (!found) {
        std::cerr << "[Compute] Compute node with ID " << argv[1] << " not found in the config\n";
        return 1;
    }

    vector<NodeInfo> controller_nodes = getControllerNodes("config/cluster_config.yaml");
    NodeInfo controllerInfo = controller_nodes[0]; // Choosing the first controller node as of now
    bool registered = registerWithController(selfInfo, controllerInfo);

    if (!registered) {
        return 1;
    }

    int server_sock = start_server(selfInfo.port);
    std::cout << "[Compute] Listening on port " << selfInfo.port << "...\n";

    std::thread reaper(reap_zombie_processes);
    reaper.detach();

    while (true) {
        int sock = accept_connection(server_sock);
        std::thread t(handle_controller, sock);
        t.detach();
    }
    return 0;
}
