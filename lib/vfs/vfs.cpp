//
// Created by maxime on 3/1/25.
//

#include "vfs.hpp"

void VirtualFileSystem::add_file(const std::string &path, const unsigned char *data, unsigned int len)  {
    const std::vector<unsigned char> file_data(data, data + len);
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