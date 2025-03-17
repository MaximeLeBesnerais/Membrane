// Membrane - A C++ and Web Tech Interface
// Created by Maxime Le Besnerais
// Copyright (c) 2025 Maxime Le Besnerais

#include "HttpServer.hpp"
#include <arpa/inet.h>
#include <poll.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

HttpServer::HttpServer(const int port)
    : server_fd(-1), port(port), running(false) {}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::mount_vfs(const std::string &prefix,
                           const VirtualFileSystem *vfs) {
    std::string normalized_prefix = prefix;
    if (!normalized_prefix.empty() && normalized_prefix.back() != '/')
        normalized_prefix += '/';

    if (normalized_prefix.front() != '/')
        normalized_prefix = '/' + normalized_prefix;
    mounted_vfs[normalized_prefix] = vfs;
}

void HttpServer::register_route(
    const std::string &path,
    const std::function<
        void(const std::string &,
             const std::unordered_map<std::string, std::string> &,
             const std::string &, std::string &,
             std::unordered_map<std::string, std::string> &)> &handler) {
    route_handlers[path] = handler;
}

bool HttpServer::start() {
    if (running) return false;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno)
                  << std::endl;
        return false;
    }

    constexpr int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
        0) {
        std::cerr << "Failed to set socket options: " << strerror(errno)
                  << std::endl;
        close(server_fd);
        server_fd = -1;
        return false;
    }

    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, reinterpret_cast<sockaddr *>(&address),
             sizeof(address)) < 0) {
        std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
        close(server_fd);
        server_fd = -1;
        return false;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "Failed to listen on socket: " << strerror(errno)
                  << std::endl;
        close(server_fd);
        server_fd = -1;
        return false;
    }

    running = true;
    accept_thread = std::thread(&HttpServer::accept_connections, this);

    for (int i = 0; i < NUM_WORKER_THREADS; i++) {
        worker_threads.emplace_back([this]() {
            while (running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }
    return true;
}

void HttpServer::stop() {
    if (!running) {
        return;
    }
    running = false;
    if (server_fd >= 0) {
        shutdown(server_fd, SHUT_RDWR);
        close(server_fd);
        server_fd = -1;
    }
    if (accept_thread.joinable()) {
        accept_thread.join();
    }
    for (auto &thread : worker_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    worker_threads.clear();
}

bool HttpServer::is_running() const {
    return running;
}

void HttpServer::accept_connections() {
    while (running) {
        ClientConnection client;
        client.addr_len = sizeof(client.address);

        client.socket =
            accept(server_fd, reinterpret_cast<sockaddr *>(&client.address),
                   &client.addr_len);

        if (client.socket < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            std::cerr << "Failed to accept connection: " << strerror(errno)
                      << std::endl;
            continue;
        }
        if (!set_nonblocking(client.socket)) {
            close(client.socket);
            continue;
        }
        handle_client(client);
    }
}

void HttpServer::handle_client(ClientConnection client) {
    std::thread client_thread([this, client]() {
        if (HttpRequest request; parse_request(client.socket, request)) {
            process_request(client.socket, request);
        }
        close(client.socket);
    });
    client_thread.detach();
}

bool HttpServer::parse_request(const int client_socket, HttpRequest &request) {
    char buffer[4096];
    std::string request_str;
    bool headers_complete = false;
    size_t content_length = 0;

    pollfd pfd;
    pfd.fd = client_socket;
    pfd.events = POLLIN;

    while (true) {
        if (const int poll_result = poll(&pfd, 1, 5000); poll_result < 0) {
            std::cerr << "Poll failed: " << strerror(errno) << std::endl;
            return false;
        } else if (poll_result == 0) {
            std::cerr << "Client connection timed out" << std::endl;
            return false;
        }

        const ssize_t bytes_read =
            recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                return false;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            std::cerr << "Error reading from socket: " << strerror(errno)
                      << std::endl;
            return false;
        }

        buffer[bytes_read] = '\0';

        request_str.append(buffer, bytes_read);

        if (!headers_complete) {
            if (const auto header_end = request_str.find("\r\n\r\n");
                header_end != std::string::npos) {
                std::istringstream stream(request_str.substr(0, header_end));
                std::string line;

                if (std::getline(stream, line)) {
                    const size_t method_end = line.find(' ');

                    if (const size_t path_end = line.find(' ', method_end + 1);
                        method_end != std::string::npos &&
                        path_end != std::string::npos) {
                        request.method = line.substr(0, method_end);
                        request.path = line.substr(method_end + 1,
                                                   path_end - method_end - 1);
                        request.version = line.substr(
                            path_end + 1, line.length() - path_end - 1);

                        if (!request.version.empty() &&
                            request.version.back() == '\r') {
                            request.version.pop_back();
                        }
                    } else {
                        std::cerr << "Malformed request line: " << line
                                  << std::endl;
                        return false;
                    }
                }

                while (std::getline(stream, line) && !line.empty()) {
                    if (!line.empty() && line.back() == '\r') {
                        line.pop_back();
                    }

                    if (line.empty()) break;

                    if (const size_t colon_pos = line.find(':');
                        colon_pos != std::string::npos) {
                        std::string key = line.substr(0, colon_pos);
                        std::string value = line.substr(colon_pos + 1);

                        value.erase(0, value.find_first_not_of(" \t"));

                        std::ranges::transform(key, key.begin(), ::tolower);

                        request.headers[key] = value;

                        if (key == "content-length") {
                            try {
                                content_length = std::stoul(value);
                            } catch ([[maybe_unused]] const std::exception &e) {
                                std::cerr << "Invalid Content-Length: " << value
                                          << std::endl;
                                return false;
                            }
                        }
                    }
                }
                request.body = request_str.substr(header_end + 4);
                headers_complete = true;
                if (request.body.length() >= content_length) {
                    request.body.resize(content_length);
                    break;
                }
            }
        } else {
            if (request.body.length() >= content_length) {
                request.body.resize(content_length);
                break;
            }
        }
    }

    return true;
}

void HttpServer::process_request(const int client_socket,
                                 const HttpRequest &request) {
    if (const auto handler_it = route_handlers.find(request.path);
        handler_it != route_handlers.end()) {
        std::string response_body;
        std::unordered_map<std::string, std::string> response_headers;
        handler_it->second(request.method, request.headers, request.body,
                           response_body, response_headers);
        send_response(client_socket, 200, "OK", response_headers,
                      response_body);
        return;
    }
    if (serve_file_from_vfs(client_socket, request.path)) {
        return;
    }
    const std::string not_found_body =
        "<html><body><h1>404 Not Found</h1></body></html>";
    const std::unordered_map<std::string, std::string> headers = {
        {"Content-Type", "text/html"},
        {"Content-Length", std::to_string(not_found_body.length())}};
    send_response(client_socket, 404, "Not Found", headers, not_found_body);
}

bool HttpServer::serve_file_from_vfs(const int client_socket,
                                     const std::string &path) {
    std::string normalized_path = path;
    if (normalized_path.empty() || normalized_path[0] != '/') {
        normalized_path = '/' + normalized_path;
    }

    for (const auto &[prefix, vfs] : mounted_vfs) {
        if (normalized_path.compare(0, prefix.length(), prefix) == 0) {
            std::string vfs_path = normalized_path.substr(prefix.length());
            if (vfs_path.empty() || vfs_path == "/") vfs_path = "index.html";

            if (!vfs_path.empty() && vfs_path[0] == '/')
                vfs_path = vfs_path.substr(1);

            if (vfs->exists(vfs_path)) {
                if (const VirtualFileSystem::FileEntry *file =
                        vfs->get_file(vfs_path)) {
                    std::unordered_map<std::string, std::string> headers = {
                        {"Content-Type", file->mime_type},
                        {"Content-Length", std::to_string(file->data.size())}};

                    std::string response = "HTTP/1.1 200 OK\r\n";
                    for (const auto &[key, value] : headers) {
                        response += key + ": " + value + "\r\n";
                    }
                    response += "\r\n";

                    if (send(client_socket, response.c_str(), response.length(),
                             0) < 0) {
                        std::cerr << "Failed to send response headers: "
                                  << strerror(errno) << std::endl;
                        return false;
                    }

                    if (send(client_socket, file->data.data(),
                             file->data.size(), 0) < 0) {
                        std::cerr
                            << "Failed to send file data: " << strerror(errno)
                            << std::endl;
                        return false;
                    }
                    return true;
                }
            }
        }
    }

    return false;
}

void HttpServer::send_response(
    const int client_socket, const int status_code,
    const std::string &status_message,
    const std::unordered_map<std::string, std::string> &headers,
    const std::string &body) {
    std::string response = "HTTP/1.1 " + std::to_string(status_code) + " " +
                           status_message + "\r\n";

    for (const auto &[key, value] : headers) {
        response += key + ": " + value + "\r\n";
    }

    if (!headers.contains("Content-Length")) {
        response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
    }

    response += "\r\n";
    response += body;

    if (send(client_socket, response.c_str(), response.length(), 0) < 0) {
        std::cerr << "Failed to send response: " << strerror(errno)
                  << std::endl;
    }
}

std::string HttpServer::get_status_message(const int status_code) {
    switch (status_code) {
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 204:
            return "No Content";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 503:
            return "Service Unavailable";
        default:
            return "Unknown";
    }
}

bool HttpServer::set_nonblocking(const int socket) {
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags < 0) {
        std::cerr << "Failed to get socket flags: " << strerror(errno)
                  << std::endl;
        return false;
    }

    flags |= O_NONBLOCK;
    if (fcntl(socket, F_SETFL, flags) < 0) {
        std::cerr << "Failed to set socket to non-blocking: " << strerror(errno)
                  << std::endl;
        return false;
    }

    return true;
}