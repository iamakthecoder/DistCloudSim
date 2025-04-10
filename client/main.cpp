// client/main.cpp
#include <iostream>
#include <string>
#include <sstream>
#include "../common/rpc.h"
#include "../common/config_parser.h"

const std::string CONTROLLER_IP = "127.0.0.1"; // for now
const int CONTROLLER_PORT = 5000; // for now

using namespace std;

void show_menu() {
    std::cout << "\nAvailable commands:\n";
    std::cout << "1. create_vm <image_name>\n";
    std::cout << "2. list_vms\n";
    std::cout << "3. terminate_vm <vm_id>\n";
    std::cout << "4. list_images\n";
    std::cout << "5. exit\n";
}

int main() {
    std::string command;
    std::cout << "[Client] Cloud Simulation Client Started\n";

    while (true) {
        show_menu();
        while(command.empty()){
            std::cout << "> ";
            std::getline(std::cin, command);
        }

        if (command.rfind("create_vm", 0) == 0) {
            std::istringstream iss(command);
            std::string cmd, image_name;
            iss >> cmd >> image_name;

            if(image_name.empty()){
                std::cout << "[Client] Usage: create_vm <image_name>\n";
                command = ""; // Clear command for next iteration
                continue;
            }

            RPCRequest req;
            req.command = "create_vm";
            req.payload = image_name;
            RPCResponse res = send_request(CONTROLLER_IP, CONTROLLER_PORT, req);
            if (res.success) {
                std::cout << "[Client] VM Created with ID: " << res.data << "\n";
            } else {
                std::cout << "[Client] Failed to create VM: " << res.data << "\n";
            }
        }
        else if (command == "list_vms") {
            RPCRequest req;
            req.command = "list_vms";
            req.payload = "";
            RPCResponse res = send_request(CONTROLLER_IP, CONTROLLER_PORT, req);
            if (res.success) {
                std::cout << "[Client] Active VMs:\n" << res.data;
            } else {
                std::cout << "[Client] Error: " << res.data << "\n";
            }
        }
        else if (command.rfind("terminate_vm", 0) == 0) {
            std::istringstream iss(command);
            std::string cmd, vm_id;
            iss >> cmd >> vm_id;
        
            if (vm_id.empty()) {
                std::cout << "[Client] Usage: terminate_vm <vm_id>\n";
                continue;
            }
        
            RPCRequest req;
            req.command = "terminate_vm";
            req.payload = vm_id;
            RPCResponse res = send_request(CONTROLLER_IP, CONTROLLER_PORT, req);
        
            if (res.success) {
                std::cout << "[Client] VM " << vm_id << " terminated successfully.\n";
            } else {
                std::cout << "[Client] Failed to terminate VM: " << res.data << "\n";
            }
        }
        else if (command == "list_images") {
            RPCRequest req;
            req.command = "list_images";
            req.payload = "";
            RPCResponse res = send_request(CONTROLLER_IP, CONTROLLER_PORT, req);
        
            if (res.success) {
                std::cout << "[Client] Available Images:\n" << res.data;
            } else {
                std::cout << "[Client] Error: " << res.data << "\n";
            }
        }
        else if (command == "exit") {
            break;
        }
        else {
            std::cout << "[Client] Unknown command\n";
        }
        command = ""; // Clear command for next iteration
    }
    return 0;
}
