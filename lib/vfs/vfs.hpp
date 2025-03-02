#pragma once
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <filesystem>
#include <iostream>

class VirtualFileSystem {
public:
    struct FileEntry {
        std::vector<unsigned char> data;
        std::string mime_type;
    };

    VirtualFileSystem() : enable_persistence(false) {}
    explicit VirtualFileSystem(std::string persistence_dir)
    : enable_persistence(true),
    persistence_dir(std::move(persistence_dir)) {
        if (!std::filesystem::exists(persistence_dir) && !std::filesystem::create_directory(persistence_dir)) {
            throw std::runtime_error("Failed to create persistence directory");
        }
        if (!load_from_disk()) {
            std::cerr << "Failed to load files from disk" << std::endl;
        }
    }
    ~VirtualFileSystem() {
        if (enable_persistence) {
            if (!save_to_disk()) {
                std::cerr << "Failed to save files to disk" << std::endl;
            }
        }
    }

    void add_file(const std::string& path, const unsigned char* data, unsigned int len);
    [[nodiscard]] bool exists(const std::string& path) const;
    [[nodiscard]] const FileEntry* get_file(const std::string& path) const;
    // persistence functions
    void set_persistence_dir(const std::string& dir) { persistence_dir = dir; }
    bool save_to_disk();
    // load from disk
    [[nodiscard]] bool load_from_disk();
    // get current files
    [[nodiscard]] const std::map<std::string, FileEntry>& get_files() const { return files; }
    bool is_persistent() const { return enable_persistence; }

private:
    const bool enable_persistence;
    std::map<std::string, FileEntry> files;
    std::string persistence_dir;
    static std::string get_mime_type(const std::string& path);
};
