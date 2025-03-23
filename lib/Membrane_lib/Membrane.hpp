#ifndef MEMBRANE_HPP
#define MEMBRANE_HPP
#include <webview/webview.h>
#include "FunctionRegistry.hpp"
#include "HttpServer.hpp"
#include <Json/json.hpp>
#include "vfs.hpp"
#include "zlib.h"
// -- Membrane Includes
#include "MembraneUtils.hpp"

using json = json_::Value;

json retObj(std::string status, std::string message,
    const std::string &data = "");


/**
 * @brief The main class of the Membrane library
 * @brief Membrane is a high-level C++ wrapper around the webview library
 * @brief It provides a simple way to create a webview application with a
 * built-in HTTP server
 * @brief and a virtual file system
 * @brief It also provides a way to register functions that can be called from
 * the webview, allowing users to leverage C++ code from JavaScript
 */
class Membrane {
public:
    // --------------------------------
    // Lifecycle Management
    // --------------------------------
    explicit Membrane(const std::string &title = "MembraneApp",
                      const std::string &entry = "index.html", int width = 800,
                      int height = 600,
                      webview_hint_t hints = WEBVIEW_HINT_NONE);
    ~Membrane() = default;

    bool run();
    void Terminate();

    // --------------------------------
    // HTTP Server Management
    // --------------------------------
    int findAvailablePort();
    
    void register_endpoint_handler(
        const std::string &endpoint_path,
        const std::function<
            void(const std::string &,
                 const std::unordered_map<std::string, std::string> &,
                 const std::string &, std::string &,
                 std::unordered_map<std::string, std::string> &)> &handler);

    // --------------------------------
    // VFS (Virtual File System) Management
    // --------------------------------
    void add_vfs(const std::string &path, const unsigned char *data,
                 unsigned int len);
                 
    void add_custom_vfs(const std::string &name);
    
    void add_persistent_vfs(const std::string &name, const std::string &path);
    
    void add_to_custom_vfs(const std::string &vfs_name, const std::string &path,
                           const unsigned char *data, unsigned int len);
    
    bool save_vfs_to_disk(const std::string &vfs_name);
    
    bool save_all_vfs_to_disk();
    
    void setDefaultVfsPath(const std::string &path) {
        _default_vfs_path = get_app_data_directory(path);
    }
    
    std::map<std::string, VirtualFileSystem::FileEntry>get_files(std::string vfs_name) {
        auto &vfs = _custom_vfs[vfs_name];
        return vfs->get_allFiles();
    };
    
    VirtualFileSystem::FileEntry get_file(std::string vfs_name, std::string path) {
        auto &vfs = _custom_vfs[vfs_name];
        return vfs->getFile(path);
    };

    // --------------------------------
    // Data Management and Compression
    // --------------------------------
    void UnzipData(const std::string &zip_path,
                   const VirtualFileSystem::FileEntry &file_entry);
                   
    void checkAndUnzip();

    // --------------------------------
    // Function Registry and JavaScript Bridge
    // --------------------------------
    void registerFunction(const std::string &name,
                          const std::function<json(const json &args)> &func);
                          
    template <typename... Args>
    void registerSimpleFunction(const std::string &name,
                                std::function<json(Args...)> func);
    std::vector<std::string> getRegisteredFunctions() {
        return _functionRegistry.getRegisteredFunctions();
    }
                                
    json callFunction(const std::string &name, const json &args);

    // --------------------------------
    // Miscellaneous
    // --------------------------------
    void setTools();
    void registerFileSystemFunctions();

private:
    // --------------------------------
    // Private Helper Methods
    // --------------------------------
    template <typename... Args, size_t... I>
    static json callWithJsonArgsHelper(std::function<json(Args...)> func,
                                       const json &args,
                                       std::index_sequence<I...>);

    template <typename... Args>
    static json callWithJsonArgs(std::function<json(Args...)> func,
                                 const json &args);

    // --------------------------------
    // Member Variables
    // --------------------------------
    std::string _default_vfs_path;
    int _port = 0;
    webview::webview _window;
    HttpServer _server;
    VirtualFileSystem _vfs;
    std::unordered_map<std::string, std::unique_ptr<VirtualFileSystem>> _custom_vfs;
    bool _running = false;
    FunctionRegistry _functionRegistry;
    std::string _entry;
};
#endif  // MEMBRANE_HPP