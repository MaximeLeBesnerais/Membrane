// Membrane - A C++ and Web Tech Interface
// Created by Maxime Le Besnerais
// Copyright (c) 2025 Maxime Le Besnerais

#include "Membrane.hpp"
#include <miniz.h>
#include <fstream>
#include <iostream>

#ifdef DEV_MODE
#define S_VURL VITE_DEV_SERVER_URL
#else
#define S_VURL ("http://localhost:" + std::to_string(_port) + "/" + _entry)
#endif

int Membrane::findAvailablePort() {
    const int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        _port = 8080;
        return 8080;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0;

    if (bind(sock, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0) {
        close(sock);
        _port = 8080;
        return 8080;
    }

    socklen_t addr_len = sizeof(addr);
    if (getsockname(sock, reinterpret_cast<struct sockaddr *>(&addr),
                    &addr_len) < 0) {
        close(sock);
        _port = 8080;
        return 8080;
    }

    const int port = ntohs(addr.sin_port);
    close(sock);
    _port = port;
    return port;
}

json retObj(std::string status, std::string message,
            const std::string &data = "") {
    return json({{"status", status}, {"message", message}, {"data", data}});
}

Membrane::Membrane(const std::string &title, const std::string &entry,
                   const int width, const int height,
                   const webview_hint_t hints)
    : _window(true, nullptr), _server(findAvailablePort()), _entry(entry) {
    _server.mount_vfs("/", &_vfs);
    if (!_server.start()) {
        std::cerr << "Failed to start HTTP server" << std::endl;
        _running = false;
        return;
    }
    _running = true;
    _window.set_title(title);
    _window.set_size(width, height, hints);
    setTools();
    setDefaultVfsPath(title);
}

void Membrane::Membrase() {
    _server.stop();
    _running = false;
    save_all_vfs_to_disk();
    _window.terminate();
}

bool Membrane::run() {
    if (!_running) {
        return false;
    }
    _window.navigate(S_VURL);
    _window.run();
    return true;
}

void Membrane::UnzipData(const std::string &zip_path,
                         const VirtualFileSystem::FileEntry &file_entry) {
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));

    if (!mz_zip_reader_init_mem(&zip, file_entry.data.data(),
                                file_entry.data.size(), 0)) {
        std::cerr << "Failed to open ZIP archive: " << zip_path << std::endl;
        return;
    }

    int file_count = mz_zip_reader_get_num_files(&zip);

    for (int i = 0; i < file_count; i++) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip, i, &file_stat)) {
            std::cerr << "Failed to get file stat for entry " << i << std::endl;
            continue;
        }

        size_t file_size = file_stat.m_uncomp_size;
        std::vector<unsigned char> extracted_data(file_size);

        if (!mz_zip_reader_extract_to_mem(&zip, i, extracted_data.data(),
                                          file_size, 0)) {
            std::cerr << "Failed to extract file: " << file_stat.m_filename
                      << std::endl;
            continue;
        }
        std::string path = file_stat.m_filename;
        path = path.substr(path.find_first_of('/') + 1);
        _vfs.add_file(path, extracted_data.data(), extracted_data.size());
    }

    mz_zip_reader_end(&zip);
}

void Membrane::checkAndUnzip() {
    auto file_list = _vfs.get_files();
    for (const auto &[path, entry] : file_list) {
        if (entry.mime_type == "application/zip") UnzipData(path, entry);
    }
}

void Membrane::add_vfs(const std::string &path, const unsigned char *data,
                       const unsigned int len) {
    _vfs.add_file(path, data, len);
}

void Membrane::openExternal(const std::string &url) {
#ifdef __APPLE__
    if (system(("open " + url).c_str()) < 0) {
        std::cerr << "Failed to open URL: " << url << std::endl;
    }
#elif __linux__
    if (system(("xdg-open " + url).c_str()) < 0) {
        std::cerr << "Failed to open URL: " << url << std::endl;
    }
#elif _WIN32
    if (system(("start " + url).c_str()) < 0) {
        std::cerr << "Failed to open URL: " << url << std::endl;
    }
#endif
}

void Membrane::saveFile(const std::string &path, const std::string &content) {
    std::ofstream file(path, std::ios::out | std::ios::binary);
    file.write(content.c_str(), static_cast<long>(content.size()));
    file.close();
}

void Membrane::registerFunction(const std::string &name,
                                const std::function<json(const json &)> &func) {
    _functionRegistry.registerFunction(name, func);

    _window.bind(name, [this, name](const std::string &args) {
        try {
            const json jArgs = json::parse(args);
            const json result = _functionRegistry.callFunction(name, jArgs);
            return result.dump();
        } catch (const std::exception &e) {
            return retObj("error", e.what()).dump();
        }
    });
}

template <typename... Args>
void Membrane::registerSimpleFunction(const std::string &name,
                                      std::function<json(Args...)> func) {
    registerFunction(name, [func](const json &args) {
        return callWithJsonArgs(func, args);
    });
}

json Membrane::callFunction(const std::string &name, const json &args) {
    return _functionRegistry.callFunction(name, args);
}

template <typename... Args, size_t... I>
json Membrane::callWithJsonArgsHelper(std::function<json(Args...)> func,
                                      const json &args,
                                      std::index_sequence<I...>) {
    if (args.size() != sizeof...(Args))
        return retObj("error", "Invalid number of arguments");
    try {
        return func(args[I].template get<std::remove_cvref_t<Args>>()...);
    } catch (const std::exception &e) {
        return retObj("error", e.what());
    }
}

template <typename... Args>
json Membrane::callWithJsonArgs(std::function<json(Args...)> func,
                                const json &args) {
    return callWithJsonArgsHelper<Args...>(func, args,
                                           std::index_sequence_for<Args...>{});
}

void Membrane::add_custom_vfs(const std::string &name) {
    if (_custom_vfs.contains(name)) {
        std::cerr << "Custom VFS with name " << name << " already exists"
                  << std::endl;
        return;
    }
    auto vfs = std::make_unique<VirtualFileSystem>();
    _server.mount_vfs("/" + name, vfs.get());
    _custom_vfs[name] = std::move(vfs);
}

void Membrane::add_persistent_vfs(const std::string &name,
                                  const std::string &path) {
    if (_custom_vfs.contains(name)) {
        std::cerr << "Custom VFS with name " << name << " already exists"
                  << std::endl;
        return;
    }
    auto vfs =
        std::make_unique<VirtualFileSystem>(_default_vfs_path + "/" + path);
    if (!vfs->load_from_disk())
        std::cerr << "Failed to load files from disk" << std::endl;
    _server.mount_vfs("/" + name, vfs.get());
    _custom_vfs[name] = std::move(vfs);
}

void Membrane::add_to_custom_vfs(const std::string &vfs_name,
                                 const std::string &path,
                                 const unsigned char *data, unsigned int len) {
    const auto found = _custom_vfs.find(vfs_name);
    if (found == _custom_vfs.end()) {
        std::cerr << "Custom VFS with name " << vfs_name << " not found"
                  << std::endl;
        return;
    }
    found->second->add_file(path, data, len);
}

void Membrane::register_endpoint_handler(
    const std::string &endpoint_path,
    const std::function<
        void(const std::string &,
             const std::unordered_map<std::string, std::string> &,
             const std::string &, std::string &,
             std::unordered_map<std::string, std::string> &)> &handler) {
    _server.register_route(endpoint_path, handler);
}

bool Membrane::save_vfs_to_disk(const std::string &vfs_name) {
    const auto vfs = _custom_vfs.find(vfs_name);
    if (vfs == _custom_vfs.end()) {
        std::cerr << "Custom VFS with name " << vfs_name << " not found"
                  << std::endl;
        return false;
    }
    return vfs->second->save_to_disk();
}

bool Membrane::save_all_vfs_to_disk() {
    bool all_success = true;
    std::vector<std::string> failed_vfs;
    for (const auto &[name, vfs] : _custom_vfs) {
        if (!vfs->is_persistent()) continue;
        if (!vfs->save_to_disk()) {
            all_success = false;
            failed_vfs.push_back(name);
        }
    }
    if (!all_success) {
        std::cerr << "Failed to save the following custom VFS to disk:"
                  << std::endl;
        for (const auto &name : failed_vfs) {
            std::cerr << "  - " << name << std::endl;
        }
    }
    return all_success;
}

void Membrane::setTools() {
    // Group 1: External URL handling
    registerFunction("openExternalUrl", [this](const json &args) {
        if (args.size() != 1)
            return retObj("error", "Invalid number of arguments");
        const std::string url = args[0].get<std::string>();
        try {
            openExternal(url);
            return retObj("success", "Opened external URL");
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });

    // Group 2: File operations
    registerFunction("saveFile", [this](const json &args) {
        if (args.size() != 3)
            return retObj("error", "Invalid number of arguments");
        const std::string path = args[0].get<std::string>();
        const std::string content = args[1].get<std::string>();
        const std::string mime = args[2].get<std::string>();
        try {
            saveFile(path, content);
            return retObj("success", "Saved file");
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });

    // Group 3: VFS creation
    registerFunction("createCustomVfs", [this](const json &args) {
        if (args.size() != 1 && args.size() != 2)
            return retObj("error",
                          "Invalid number of arguments. Expected 1 or 2 "
                          "arguments: vfs_name, "
                          "[persistence_dir]");

        const std::string vfs_name = args[0].get<std::string>();
        try {
            if (args.size() == 2) {
                const std::string persistence_dir = args[1].get<std::string>();
                add_persistent_vfs(vfs_name, persistence_dir);
            } else {
                add_custom_vfs(vfs_name);
            }
            return retObj("success", "Created custom VFS: " + vfs_name);
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });

    // Group 4: VFS file operations
    registerFunction("addFileToVfs", [this](const json &args) {
        if (args.size() != 3)
            return retObj("error",
                          "Invalid number of arguments. Expected 3 arguments: "
                          "vfs_name, path, content");

        const std::string vfs_name = args[0].get<std::string>();
        const std::string path = args[1].get<std::string>();
        const std::string content = args[2].get<std::string>();

        try {
            const std::vector<unsigned char> data(content.begin(),
                                                  content.end());
            add_to_custom_vfs(vfs_name, path, data.data(), data.size());
            return retObj("success",
                          "Added file to VFS: " + vfs_name + "/" + path);
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });

    // Group 5: VFS persistence
    registerFunction("saveVfsToDisk", [this](const json &args) {
        if (args.size() != 1) {
            return retObj(
                "error",
                "Invalid number of arguments. Expected 1 argument: vfs_name");
        }

        const std::string vfs_name = args[0].get<std::string>();

        try {
            if (save_vfs_to_disk(vfs_name)) {
                return retObj("success", "Saved VFS to disk: " + vfs_name);
            }
            return retObj("error", "Failed to save VFS to disk: " + vfs_name);
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });

    registerFunction("saveAllVfsToDisk", [this](const json &) {
        try {
            if (save_all_vfs_to_disk()) {
                return retObj("success", "Saved all VFS instances to disk");
            }
            return retObj("error", "Failed to save some VFS instances to disk");
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });

    // All JavaScript bindings consolidated in one place
    _window.eval(R"script(
        // File operations
        window.saveFile = async (path, content, mime) => {
            return window.saveFile(path, content, mime);
        };

        // VFS namespace
        window.membrane = window.membrane || {};
        window.membrane.vfs = {
            // Create a new custom VFS
            createCustomVfs: async (name, persistenceDir = null) => {
                if (persistenceDir) {
                    return window.createCustomVfs(name, persistenceDir);
                } else {
                    return window.createCustomVfs(name);
                }
            },
            
            // Add a file to a VFS
            addFile: async (vfsName, path, content) => {
                return window.addFileToVfs(vfsName, path, content);
            },
            
            // VFS persistence operations
            saveToDisk: async (vfsName) => {
                return window.saveVfsToDisk(vfsName);
            },
            
            saveAllToDisk: async () => {
                return window.saveAllVfsToDisk();
            }
        };
    )script");
}
