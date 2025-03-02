//
// Created by maxime on 3/1/25.
//

#include "Membrane.hpp"
#include <iostream>

int Membrane::findAvailablePort() {
    const int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        _port = 8080;
        return 8080;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0;

    if (bind(sock, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0) {
        close(sock);
        _port = 8080;
        return 8080;
    }

    socklen_t addr_len = sizeof(addr);
    if (getsockname(sock, reinterpret_cast<struct sockaddr *>(&addr), &addr_len) < 0) {
        close(sock);
        _port = 8080;
        return 8080;
    }

    const int port = ntohs(addr.sin_port);
    close(sock);
    _port = port;
    return port;
}

Membrane::Membrane(const std::string &title,
    const std::string &entry,
    const int width,
    const int height,
    const webview_hint_t hints)
:_window(true, nullptr),
_server(findAvailablePort())
{
    _server.mount_vfs("/", &_vfs);
    if (!_server.start()) {
        std::cerr << "Failed to start HTTP server" << std::endl;
        _running = false;
        return;
    }
    _running = true;
    _window.set_title(title);
    _window.set_size(width, height, hints);
    #ifdef DEV_MODE
        _window.navigate(VITE_DEV_SERVER_URL);
    #else
        _window.navigate("http://localhost:" + std::to_string(_port) + "/" + entry);
    #endif
}

bool Membrane::run() {
    if (!_running) {
        return false;
    }
    _window.run();
    return true;
}

Membrane::~Membrane() {
    _server.stop();
}

void Membrane::add_vfs(const std::string &path, const unsigned char *data, const unsigned int len) {
    _vfs.add_file(path, data, len);
}



void Membrane::registerFunction(const std::string &name, std::function<json(const json&)> func) {
    _functionRegistry.registerFunction(name, func);

    _window.bind(name, [this, name](const std::string &args) {
        try {
            const json jArgs = json::parse(args);
            const json result = _functionRegistry.callFunction(name, jArgs);
            return result.dump();
        }
        catch (const std::exception &e) {
            const json err = {
                {"status", "error"},
                {"message", e.what()},
                {"data", nullptr}
            };
            return err.dump();
        }
    });
}

template<typename... Args>
void Membrane::registerSimpleFunction(const std::string &name, std::function<json(Args...)> func) {
    registerFunction(name, [func](const json &args) {
        return callWithJsonArgs(func, args);
    });
}

json Membrane::callFunction(const std::string &name, const json &args) {
    return _functionRegistry.callFunction(name, args);
}

template<typename... Args, size_t... I>
json Membrane::callWithJsonArgsHelper(std::function<json(Args...)> func, const json &args, std::index_sequence<I...>) {
    if (args.size() != sizeof...(Args))
        return {
            {"status", "error"},
            {"message", "Invalid number of arguments"},
            {"data", nullptr}
        };
    try {
        return func(args[I].template get<std::remove_cvref_t<Args>>()...);
    }
    catch (const std::exception &e) {
        return {
            {"status", "error"},
            {"message", e.what()},
            {"data", nullptr}
        };
    }
}

template<typename... Args>
json Membrane::callWithJsonArgs(std::function<json(Args...)> func, const json &args) {
    return callWithJsonArgsHelper<Args...>(func, args, std::index_sequence_for<Args...>{});
}
