//
// Created by maxime on 3/1/25.
//

#include "vfs.hpp"

#include <algorithm>
#include <iostream>
#include <filesystem>
#include <fstream>

void VirtualFileSystem::add_file(const std::string &path, const unsigned char *data, const unsigned int len)  {
    const std::vector file_data(data, data + len);
    files[path] = {file_data, get_mime_type(path)};
}

bool VirtualFileSystem::exists(const std::string &path) const {
    return files.contains(path);
}

const VirtualFileSystem::FileEntry *VirtualFileSystem::get_file(const std::string &path) const {
    const auto it = files.find(path);
    if (it != files.end()) {
        return &it->second;
    }
    return nullptr;
}

std::string VirtualFileSystem::get_mime_type(const std::string &path) {
    if (path.ends_with(".html")) return "text/html";
    if (path.ends_with(".css")) return "text/css";
    if (path.ends_with(".js")) return "application/javascript";
    if (path.ends_with(".png")) return "image/png";
    if (path.ends_with(".jpg") || path.ends_with(".jpeg")) return "image/jpeg";
    if (path.ends_with(".svg")) return "image/svg+xml";
    if (path.ends_with(".json")) return "application/json";
    if (path.ends_with(".wasm")) return "application/wasm";
    return "application/octet-stream";
}

bool VirtualFileSystem::save_to_disk() {
    if (!enable_persistence) {
        std::cerr << "Persistence is not enabled" << std::endl;
        return false;
    }
    if (persistence_dir.empty()) {
        std::cerr << "Persistence directory is not set" << std::endl;
        return false;
    }
    if (!std::filesystem::exists(persistence_dir) && !std::filesystem::create_directory(persistence_dir)) {
        std::cerr << "Failed to create persistence directory" << std::endl;
        return false;
    }
    for (const auto& [path, entry] : files) {
        std::filesystem::path path_on_disk = persistence_dir + "/" + path;
        if (!std::filesystem::exists(path_on_disk.parent_path()) && !std::filesystem::create_directories(path_on_disk.parent_path())) {
            std::cerr << "Failed to create parent directory for " << path_on_disk << std::endl;
            return false;
        }
        std::ofstream file(path_on_disk, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file " << path_on_disk << std::endl;
            return false;
        }
        file.write(reinterpret_cast<const char*>(entry.data.data()), static_cast<long>(entry.data.size()));
        file.close();
    }
    return true;
}

// this function opens the persisted files and loads them into memory
bool VirtualFileSystem::load_from_disk() {
    if (!enable_persistence) {
        std::cerr << "Persistence is not enabled" << std::endl;
        return false;
    }
    if (persistence_dir.empty()) {
        std::cerr << "Persistence directory is not set" << std::endl;
        return false;
    }
    if (!std::filesystem::exists(persistence_dir)) {
        std::cerr << "Persistence directory does not exist" << std::endl;
        return false;
    }
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(persistence_dir)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const std::filesystem::path& path = entry.path();
            std::string relative_path = path.lexically_relative(persistence_dir).string();
            std::ranges::replace(relative_path, '\\', '/');

            std::ifstream file_stream(path, std::ios::binary);
            if (!file_stream.is_open()) {
                std::cerr << "Failed to open file " << path << std::endl;
                return false;
            }

            file_stream.seekg(0, std::ios::end);
            const auto file_size = file_stream.tellg();
            file_stream.seekg(0, std::ios::beg);

            std::vector<unsigned char> file_data(file_size);
            file_stream.read(reinterpret_cast<char*>(file_data.data()), file_size);
            file_stream.close();

            files[relative_path] = {file_data, get_mime_type(relative_path)};
        }
        return true;
    }
    catch (const std::exception &e) {
        std::cerr << "Failed to load files from disk: " << e.what() << std::endl;
        return false;
    }
}
