//
// Created by maxime on 3/1/25.
//

#include "Membrane.hpp"

Membrane::Membrane(const std::string &title,
    const std::string &entry,
    const int width,
    const int height,
    const webview_hint_t hints)
:_window(true, nullptr),
_server(8080)
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
    _window.navigate("http://localhost:8080/" + entry);
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
