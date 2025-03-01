#pragma once
#include <map>
#include <string>
#include <vector>

class VirtualFileSystem {
public:
    struct FileEntry {
        std::vector<unsigned char> data;
        std::string mime_type;
    };

    VirtualFileSystem() {};

    void add_file(const std::string& path, const unsigned char* data, unsigned int len);

    bool exists(const std::string& path) const;

    const FileEntry* get_file(const std::string& path) const;

private:
    std::map<std::string, FileEntry> files;
    
    static std::string get_mime_type(const std::string& path) {
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
};
