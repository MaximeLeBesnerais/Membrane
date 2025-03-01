//
// Created by maxime on 3/1/25.
//

#ifndef MEMBRANE_HPP
    #define MEMBRANE_HPP
    #include "vfs.hpp"
    #include "HttpServer.hpp"
    #include <webview/webview.h>
    #include <iostream>
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
    private:
        int _port = 0;
        webview::webview _window;
        HttpServer _server;
        VirtualFileSystem _vfs;
        bool _running = false;
    };
#endif //MEMBRANE_HPP
