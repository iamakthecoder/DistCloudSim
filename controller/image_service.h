#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct ImageMetadata {
    std::string name;
    std::string os;
    std::vector<std::string> preinstalled;
    int cpu;
    int memory;
};

class ImageService {
public:
    bool load_images_from_disk(const std::string& dir_path);
    std::vector<std::string> list_image_names() const;
    bool get_image(const std::string& name, ImageMetadata& image) const;

private:
    std::unordered_map<std::string, ImageMetadata> images;
};
