//
// Created by maxime on 3/1/25.
//

#ifndef MEMBRANE_HPP
    #define MEMBRANE_HPP
    #include "vfs.hpp"
    #include "HttpServer.hpp"
    #include "FunctionRegistry.hpp"
    #include <webview/webview.h>
    #include "nlohmann/json.hpp"

    using json = nlohmann::json;

    class Membrane {
    public:
        explicit Membrane(const std::string &title = "MembraneApp",
                          const std::string &entry = "index.html",
                          int width = 800,
                          int height = 600,
                          webview_hint_t hints = WEBVIEW_HINT_NONE);
        ~Membrane();

        bool run();
        int findAvailablePort();
        void registerFunction(const std::string &name,
            std::string pattern,
            const std::function<json(const json& args)> &func);

        template <typename... Args>
        void registerSimpleFunction(const std::string &name,
            std::function<json(Args...)> func);
        json callFunction(const std::string &name, const json &args);
        static void openExternal(const std::string &url);
        static void saveFile(const std::string &path, const std::string &content, const std::string &mime);

        // default vfs: add files to the virtual file system
        void add_vfs(const std::string &path, const unsigned char *data, unsigned int len);

        // custom vfs
        // add a custom virtual file system
        void add_custom_vfs(const std::string &name);
        // add a custom virtual file system with persistent storage
        void add_persistent_vfs(const std::string &name, const std::string &path);
        // add files to a custom virtual file system
        void add_to_custom_vfs(const std::string &vfs_name,
            const std::string &path,
            const unsigned char *data,
            unsigned int len);
        // save vfs to disk
        bool save_vfs_to_disk(const std::string &vfs_name);
        bool save_all_vfs_to_disk();

        void register_endpoint_handler(const std::string& endpoint_path,
            const std::function<void(const std::string&,
            const std::unordered_map<std::string, std::string>&,
            const std::string&,
            std::string&,
            std::unordered_map<std::string, std::string>&)>& handler);


    private:
        int _port = 0;
        webview::webview _window;
        HttpServer _server;
        VirtualFileSystem _vfs;
        std::unordered_map<std::string, std::unique_ptr<VirtualFileSystem>> _custom_vfs;
        bool _running = false;
        FunctionRegistry _functionRegistry;

        template <typename... Args, size_t... I>
        static json callWithJsonArgsHelper(std::function<json(Args...)> func,
            const json &args, std::index_sequence<I...>);

        template <typename... Args>
        static json callWithJsonArgs(std::function<json(Args...)> func,
            const json &args);
    };
#endif //MEMBRANE_HPP
