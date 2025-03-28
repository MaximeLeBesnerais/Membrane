#include "Membrane.hpp"

json retObj(std::string status, std::string message, const std::string &data) {
    return json({{"status", status}, {"message", message}, {"data", data}});
}

std::string base64Encode(const uint8_t* data, size_t length) {
    static const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    std::string encoded;
    int i = 0;
    int j = 0;
    uint8_t char_array_3[3];
    uint8_t char_array_4[4];
    
    while (length--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for (i = 0; i < 4; i++) {
                encoded += base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }
    
    if (i) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        
        for (j = 0; j < i + 1; j++) {
            encoded += base64_chars[char_array_4[j]];
        }
        
        while (i++ < 3) {
            encoded += '=';
        }
    }
    
    return encoded;
}

std::vector<uint8_t> base64Decode(const std::string& encoded) {
    static const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    int in_len = encoded.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    uint8_t char_array_4[4], char_array_3[3];
    std::vector<uint8_t> decoded;
    
    while (in_len-- && (encoded[in_] != '=') && 
           (isalnum(encoded[in_]) || (encoded[in_] == '+') || (encoded[in_] == '/'))) {
        char_array_4[i++] = encoded[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = base64_chars.find(char_array_4[i]);
            }
            
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            
            for (i = 0; i < 3; i++) {
                decoded.push_back(char_array_3[i]);
            }
            i = 0;
        }
    }
    
    if (i) {
        for (j = i; j < 4; j++) {
            char_array_4[j] = 0;
        }
        
        for (j = 0; j < 4; j++) {
            char_array_4[j] = base64_chars.find(char_array_4[j]);
        }
        
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
        
        for (j = 0; j < i - 1; j++) {
            decoded.push_back(char_array_3[j]);
        }
    }
    
    return decoded;
}

void Membrane::registerFileSystemFunctions() {
    // File System Operations
    registerFunction("membrane_fs_save", [this](const json &args) {
        if (args.size() != 2)
            return retObj("error", "Invalid number of arguments");
        const std::string path = args[0].get<std::string>();
        const std::string content = args[1].get<std::string>();
        try {
            saveFile(path, content);
            return retObj("success", "Saved file");
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });

    registerFunction("membrane_fs_read", [this](const json &args) {
        if (args.size() != 1) {
            return retObj("error", "Invalid number of arguments. Expected 1: path");
        }
        
        const std::string path = args[0].get<std::string>();
        
        try {
            std::string content = readFile(path);
            return json({
                {"status", "success"},
                {"message", "File read successfully"},
                {"data", content}
            });
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });
    
    registerFunction("membrane_fs_exists", [this](const json &args) {
        if (args.size() != 1) {
            return retObj("error", "Invalid number of arguments. Expected 1: path");
        }
        
        const std::string path = args[0].get<std::string>();
        
        try {
            bool exists = fileExists(path);
            return json({
                {"status", "success"},
                {"message", exists ? "File exists" : "File does not exist"},
                {"data", exists}
            });
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });
    
    registerFunction("membrane_fs_listDir", [this](const json &args) {
        if (args.size() != 1) {
            return retObj("error", "Invalid number of arguments. Expected 1: path");
        }
        
        const std::string path = args[0].get<std::string>();
        
        try {
            std::vector<std::string> entries = listDirectory(path);
            return json({
                {"status", "success"},
                {"message", "Directory listed successfully"},
                {"data", entries}
            });
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });
    
    registerFunction("membrane_fs_createDir", [this](const json &args) {
        if (args.size() != 1) {
            return retObj("error", "Invalid number of arguments. Expected 1: path");
        }
        
        const std::string path = args[0].get<std::string>();
        
        try {
            bool created = createDirectory(path);
            return json({
                {"status", "success"},
                {"message", created ? "Directory created" : "Directory already exists"},
                {"data", created}
            });
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });
    
    registerFunction("membrane_fs_copy", [this](const json &args) {
        if (args.size() != 2) {
            return retObj("error", "Invalid number of arguments. Expected 2: source, destination");
        }
        
        const std::string source = args[0].get<std::string>();
        const std::string destination = args[1].get<std::string>();
        
        try {
            copyFile(source, destination);
            return retObj("success", "File/directory copied successfully");
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });
    
    registerFunction("membrane_fs_delete", [this](const json &args) {
        if (args.size() != 1) {
            return retObj("error", "Invalid number of arguments. Expected 1: path");
        }
        
        const std::string path = args[0].get<std::string>();
        
        try {
            bool deleted = deleteFileOrDirectory(path);
            return json({
                {"status", "success"},
                {"message", deleted ? "File/directory deleted" : "Nothing to delete"},
                {"data", deleted}
            });
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });
    
    registerFunction("membrane_fs_getInfo", [this](const json &args) {
        if (args.size() != 1) {
            return retObj("error", "Invalid number of arguments. Expected 1: path");
        }
        
        const std::string path = args[0].get<std::string>();
        
        try {
            json info = getFileInfo(path);
            return json({
                {"status", "success"},
                {"message", "File information retrieved"},
                {"data", info}
            });
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });
    
    registerFunction("membrane_fs_watch", [this](const json &args) {
        if (args.size() != 2) {
            return retObj("error", "Invalid number of arguments. Expected 2: path, eventName");
        }
        
        const std::string path = args[0].get<std::string>();
        const std::string eventName = args[1].get<std::string>();
        
        try {
            // Create a callback that will emit events to the webview
            auto callback = [this, eventName](const std::string& eventType, const std::string& eventPath) {
                // Create a JavaScript event dispatch call
                std::string js = "if(window.dispatchEvent) { window.dispatchEvent(new CustomEvent('" + 
                    eventName + "', { detail: { type: '" + eventType + "', path: '" + eventPath + "' } })); }";
                
                // Execute JS in webview to notify of the change
                _window.eval(js);
            };
            
            int watcherId = watchFileOrDirectory(path, callback);
            
            return json({
                {"status", "success"},
                {"message", "File/directory watch started"},
                {"data", {{"watcherId", watcherId}}}
            });
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });
    
    registerFunction("membrane_fs_readBinary", [this](const json &args) {
        if (args.size() != 1) {
            return retObj("error", "Invalid number of arguments. Expected 1: path");
        }
        
        const std::string path = args[0].get<std::string>();
        
        try {
            std::vector<uint8_t> data = readBinaryFile(path);
            
            // Convert binary data to base64 for safe JSON transport
            std::string base64Data = base64Encode(data.data(), data.size());
            
            return json({
                {"status", "success"},
                {"message", "Binary file read successfully"},
                {"data", base64Data}
            });
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });
    
    registerFunction("membrane_fs_writeBinary", [this](const json &args) {
        if (args.size() != 2) {
            return retObj("error", "Invalid number of arguments. Expected 2: path, base64Data");
        }
        
        const std::string path = args[0].get<std::string>();
        const std::string base64Data = args[1].get<std::string>();
        
        try {
            // Decode base64 data
            std::vector<uint8_t> binaryData = base64Decode(base64Data);
            
            writeBinaryFile(path, binaryData);
            
            return retObj("success", "Binary file written successfully");
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });
    
    registerFunction("membrane_fs_createTemp", [this](const json &args) {
        std::string prefix = "membrane";
        std::string extension = ".tmp";
        
        if (args.size() >= 1) {
            prefix = args[0].get<std::string>();
        }
        
        if (args.size() >= 2) {
            extension = args[1].get<std::string>();
            // Ensure extension starts with a dot
            if (!extension.empty() && extension[0] != '.') {
                extension = "." + extension;
            }
        }
        
        try {
            std::string tempFilePath = createTempFile(prefix, extension);
            
            return json({
                {"status", "success"},
                {"message", "Temporary file created"},
                {"data", tempFilePath}
            });
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });
}

void Membrane::setTools() {
    // System Operations
    registerFunction("membrane_system_openUrl", [this](const json &args) {
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

    // File system operations
    registerFileSystemFunctions();

    // VFS Operations
    registerFunction("membrane_vfs_create", [this](const json &args) {
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

    registerFunction("membrane_vfs_addFile", [this](const json &args) {
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

    registerFunction("membrane_vfs_save", [this](const json &args) {
        if (args.size() != 1) {
            return retObj("error", 
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

    registerFunction("membrane_vfs_saveAll", [this](const json &) {
        try {
            if (save_all_vfs_to_disk()) {
                return retObj("success", "Saved all VFS instances to disk");
            }
            return retObj("error", "Failed to save some VFS instances to disk");
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });

    // Clipboard Operations
    registerFunction("membrane_clipboard_write", [](const json &args) {
        if (args.size() != 1 || !args[0].is_string()) {
            return retObj("error",
                          "Expected 1 string argument for clipboard content");
        }
        const std::string content = args[0].get<std::string>();
        if (writeClipboard(content)) {
            return retObj("success", "Clipboard updated");
        } else {
            return retObj("error", "Failed to write to clipboard");
        }
    });

    registerFunction("membrane_clipboard_read", [](const json &) {
        const std::string content = readClipboard();
        if (content.empty()) {
            return retObj("error",
                          "Failed to read clipboard or clipboard is empty");
        }
        return retObj("success", "Clipboard content", content);
    });

    // Utility Operations
    registerFunction("membrane_util_listFunctions", [this](const json &) {
        std::vector<std::string>functions_list = _functionRegistry.getRegisteredFunctions();
        return json({
            {"status", "success"},
            {"message", "Registered functions"},
            {"data", functions_list},
        });
    });
    
    // Initialize JavaScript bridge with updated function names
    _window.eval(R"script(
        window.membrane = window.membrane || {};
        
        // File System API
        window.membrane.fs = {
            save: async (path, content) => window.membrane_fs_save(path, content),
            read: async (path) => window.membrane_fs_read(path),
            exists: async (path) => window.membrane_fs_exists(path),
            listDir: async (path) => window.membrane_fs_listDir(path),
            createDir: async (path) => window.membrane_fs_createDir(path),
            copy: async (source, dest) => window.membrane_fs_copy(source, dest),
            delete: async (path) => window.membrane_fs_delete(path),
            getInfo: async (path) => window.membrane_fs_getInfo(path),
            watch: async (path, eventName) => window.membrane_fs_watch(path, eventName),
            readBinary: async (path) => window.membrane_fs_readBinary(path),
            writeBinary: async (path, data) => window.membrane_fs_writeBinary(path, data),
            createTemp: async (prefix, ext) => window.membrane_fs_createTemp(prefix, ext)
        };
        
        // VFS API
        window.membrane.vfs = {
            create: async (name, persistenceDir = null) => {
                return persistenceDir ? 
                    window.membrane_vfs_create(name, persistenceDir) : 
                    window.membrane_vfs_create(name);
            },
            addFile: async (vfsName, path, content) => window.membrane_vfs_addFile(vfsName, path, content),
            save: async (vfsName) => window.membrane_vfs_save(vfsName),
            saveAll: async () => window.membrane_vfs_saveAll()
        };
        
        // System API
        window.membrane.system = {
            openUrl: async (url) => window.membrane_system_openUrl(url)
        };
        
        // Clipboard API
        window.membrane.clipboard = {
            write: async (text) => window.membrane_clipboard_write(text),
            read: async () => window.membrane_clipboard_read()
        };
        
        // Utility API
        window.membrane.util = {
            listFunctions: async () => window.membrane_util_listFunctions()
        };
    )script");
}
