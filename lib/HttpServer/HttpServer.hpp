#pragma once

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
    // Constructor - port to listen on
    HttpServer(int port);
    
    // Destructor - cleanup resources
    ~HttpServer();
    
    // Mount VFS to a specific path prefix
    void mount_vfs(const std::string& prefix, const VirtualFileSystem* vfs);
    
    // Register a handler for specific HTTP routes
    void register_route(const std::string& path,
                       const std::function<void(const std::string&,
                                         const std::unordered_map<std::string, std::string>&,
                                         const std::string&,
                                         std::string&,
                                         std::unordered_map<std::string, std::string>&)> &handler);
    
    // Start the server (non-blocking)
    bool start();
    
    // Stop the server
    void stop();
    
    // Check if server is running
    bool is_running() const;

private:
    // Represents a single client connection
    struct ClientConnection {
        int socket;
        struct sockaddr_in address;
        socklen_t addr_len;
    };
    
    // HTTP request structure
    struct HttpRequest {
        std::string method;
        std::string path;
        std::string version;
        std::unordered_map<std::string, std::string> headers;
        std::string body;
    };
    
    // Socket file descriptor
    int server_fd;
    
    // Port number
    int port;
    
    // Server running state
    std::atomic<bool> running;
    
    // Thread handling accept connections
    std::thread accept_thread;
    
    // Worker threads for handling connections
    std::vector<std::thread> worker_threads;
    
    // Number of worker threads
    static constexpr int NUM_WORKER_THREADS = 4;
    
    // Mounted virtual file systems with their mount points
    std::unordered_map<std::string, const VirtualFileSystem*> mounted_vfs;
    
    // Route handlers
    std::unordered_map<std::string, 
                      std::function<void(const std::string&, 
                                        const std::unordered_map<std::string, std::string>&,
                                        const std::string&,
                                        std::string&,
                                        std::unordered_map<std::string, std::string>&)>> route_handlers;
    
    // Method to accept connections
    void accept_connections();
    
    // Method to handle a client connection
    void handle_client(ClientConnection client);
    
    // Parse an HTTP request
    static bool parse_request(int client_socket, HttpRequest& request);
    
    // Process an HTTP request
    void process_request(int client_socket, const HttpRequest& request);
    
    // Send an HTTP response
    static void send_response(int client_socket,
                              int status_code,
                              const std::string& status_message,
                              const std::unordered_map<std::string, std::string>& headers,
                              const std::string& body);
    
    // Serve a file from VFS
    bool serve_file_from_vfs(int client_socket, const std::string& path);
    
    // Get status message for HTTP status code
    static std::string get_status_message(int status_code);
    
    // Set socket to non-blocking mode
    static bool set_nonblocking(int socket);
};