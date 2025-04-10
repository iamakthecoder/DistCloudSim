#include "image_service.h"
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

bool ImageService::load_images_from_disk(const std::string& dir_path) {
    images.clear(); // Clear existing images
    for (const auto& entry : fs::directory_iterator(dir_path)) {
        if (entry.path().extension() == ".yaml") {
            try {
                YAML::Node config = YAML::LoadFile(entry.path().string());

                ImageMetadata image;
                image.name = config["name"].as<std::string>();
                image.os = config["os"].as<std::string>();
                image.cpu = config["cpu"].as<int>();
                image.memory = config["memory"].as<int>();

                for (const auto& item : config["preinstalled"]) {
                    image.preinstalled.push_back(item.as<std::string>());
                }

                images[image.name] = image;
                std::cout << "[ImageService] Loaded image: " << image.name << "\n";

            } catch (const std::exception& e) {
                std::cerr << "[ImageService] Failed to load image: " << entry.path() 
                          << " Error: " << e.what() << "\n";
            }
        }
    }
    return !images.empty();
}

std::vector<std::string> ImageService::list_image_names() const {
    std::vector<std::string> names;
    for (const auto& [name, _] : images) {
        names.push_back(name);
    }
    return names;
}

bool ImageService::get_image(const std::string& name, ImageMetadata& image) const {
    auto it = images.find(name);
    if (it != images.end()) {
        image = it->second;
        return true;
    }
    return false;
}