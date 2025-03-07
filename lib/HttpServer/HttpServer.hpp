// Membrane - A C++ and Web Tech Interface
// Created by Maxime Le Besnerais
// Copyright (c) 2025 Maxime Le Besnerais

#ifndef HTTPSERVER_HPP
    #define HTTPSERVER_HPP
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <string>
    #include <functional>
    #include <thread>
    #include <atomic>
    #include <vector>
    #include <unordered_map>
    #include <nlohmann/json.hpp>
    #include "vfs.hpp"

    class HttpServer {
    public:
        explicit HttpServer(int port);
        ~HttpServer();
        
        void mount_vfs(const std::string& prefix, const VirtualFileSystem* vfs);
        
        void register_route(const std::string& path,
                        const std::function<void(const std::string&,
                                            const std::unordered_map<std::string, std::string>&,
                                            const std::string&,
                                            std::string&,
                                            std::unordered_map<std::string, std::string>&)> &handler);
        
        bool start();
        void stop();
        bool is_running() const;
    private:
        struct ClientConnection {
            int socket;
            sockaddr_in address;
            socklen_t addr_len;
        };
        
        struct HttpRequest {
            std::string method;
            std::string path;
            std::string version;
            std::unordered_map<std::string, std::string> headers;
            std::string body;
        };
        
        int server_fd;
        int port;
        std::atomic<bool> running;
        std::thread accept_thread;
        std::vector<std::thread> worker_threads;
        static constexpr int NUM_WORKER_THREADS = 4;
        std::unordered_map<std::string, const VirtualFileSystem*> mounted_vfs;
        std::unordered_map<std::string,
                        std::function<void(const std::string&, 
                                            const std::unordered_map<std::string, std::string>&,
                                            const std::string&,
                                            std::string&,
                                            std::unordered_map<std::string, std::string>&)>> route_handlers;
        void accept_connections();
        void handle_client(ClientConnection client);
        static bool parse_request(int client_socket, HttpRequest& request);
        void process_request(int client_socket, const HttpRequest& request);
        static void send_response(int client_socket,
                                int status_code,
                                const std::string& status_message,
                                const std::unordered_map<std::string, std::string>& headers,
                                const std::string& body);
        bool serve_file_from_vfs(int client_socket, const std::string& path);
        static std::string get_status_message(int status_code);
        static bool set_nonblocking(int socket);
    };
#endif // HTTPSERVER_HPP
