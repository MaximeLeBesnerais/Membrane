// Membrane - A C++ and Web Tech Interface
// Created by Maxime Le Besnerais
// Copyright (c) 2025 Maxime Le Besnerais

#ifndef VFS_HPP
#define VFS_HPP
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

class VirtualFileSystem {
public:
    struct FileEntry {
        std::vector<unsigned char> data;
        std::string mime_type;
    };

    VirtualFileSystem() : enable_persistence(false) {}
    explicit VirtualFileSystem(std::string persistence_dir);

    ~VirtualFileSystem() {
        if (enable_persistence) {
            if (!save_to_disk()) {
                std::cerr << "Failed to save files to disk" << std::endl;
            }
        }
    }

    void add_file(const std::string &path, const unsigned char *data,
                  unsigned int len);
    [[nodiscard]] bool exists(const std::string &path) const;
    [[nodiscard]] const FileEntry *get_file(const std::string &path) const;
    // persistence functions
    void set_persistence_dir(const std::string &dir) {
        persistence_dir = dir;
    }
    bool save_to_disk();
    // load from disk
    [[nodiscard]] bool load_from_disk();
    // get current files
    [[nodiscard]] const std::map<std::string, FileEntry> &get_files() const {
        return files;
    }
    [[nodiscard]] bool is_persistent() const {
        return enable_persistence;
    }
    inline std::map<std::string, FileEntry>get_allFiles() {return files;};

private:
    const bool enable_persistence;
    std::map<std::string, FileEntry> files;
    std::string persistence_dir;
    static std::string get_mime_type(const std::string &path);
};
#endif  // VFS_HPP
