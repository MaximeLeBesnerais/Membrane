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

        void add_vfs(const std::string &path, const unsigned char *data, unsigned int len);
        int findAvailablePort();

        void registerFunction(const std::string &name,
            std::function<json(const json& args)> func);

        template <typename... Args>
        void registerSimpleFunction(const std::string &name,
            std::function<json(Args...)> func);

        json callFunction(const std::string &name, const json &args);
    private:
        int _port = 0;
        webview::webview _window;
        HttpServer _server;
        VirtualFileSystem _vfs;
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
