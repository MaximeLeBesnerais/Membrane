#include "Membrane.hpp"

using json = json_::Value;

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
    registerFunction("membrane_saveFile", [this](const json &args) {
        if (args.size() != 2)
            return retObj("error", "Invalid number of arguments");
        const std::string path = args[0].as_string();
        const std::string content = args[1].as_string();
        try {
            saveFile(path, content);
            return retObj("success", "Saved file");
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });

    // 1. Read File Content
    registerFunction("membrane_readFile", [this](const json &args) {
        if (args.size() != 1) {
            return retObj("error", "Invalid number of arguments. Expected 1: path");
        }
        
        const std::string path = args[0].as_string();
        
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
    
    // 2. Check File Existence
    registerFunction("membrane_fileExists", [this](const json &args) {
        if (args.size() != 1) {
            return retObj("error", "Invalid number of arguments. Expected 1: path");
        }
        
        const std::string path = args[0].as_string();
        
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
    
    // 3. List Directory Contents
    registerFunction("membrane_listDirectory", [this](const json &args) {
        if (args.size() != 1) {
            return retObj("error", "Invalid number of arguments. Expected 1: path");
        }
        
        const std::string path = args[0].as_string();
        
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
    
    // 4. Create Directory
    registerFunction("membrane_createDirectory", [this](const json &args) {
        if (args.size() != 1) {
            return retObj("error", "Invalid number of arguments. Expected 1: path");
        }
        
        const std::string path = args[0].as_string();
        
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
    
    // 5. Copy File/Directory
    registerFunction("membrane_copyFile", [this](const json &args) {
        if (args.size() != 2) {
            return retObj("error", "Invalid number of arguments. Expected 2: source, destination");
        }
        
        const std::string source = args[0].as_string();
        const std::string destination = args[1].as_string();
        
        try {
            copyFile(source, destination);
            return retObj("success", "File/directory copied successfully");
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });
    
    // 6. Delete File/Directory
    registerFunction("membrane_deleteFileOrDirectory", [this](const json &args) {
        if (args.size() != 1) {
            return retObj("error", "Invalid number of arguments. Expected 1: path");
        }
        
        const std::string path = args[0].as_string();
        
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
    
    // 7. Get File Information
    registerFunction("membrane_getFileInfo", [this](const json &args) {
        if (args.size() != 1) {
            return retObj("error", "Invalid number of arguments. Expected 1: path");
        }
        
        const std::string path = args[0].as_string();
        
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
    
    // 8. Watch File/Directory for Changes
    // This implementation uses a callback system that notifies the frontend via events
    registerFunction("membrane_watchFileOrDirectory", [this](const json &args) {
        if (args.size() != 2) {
            return retObj("error", "Invalid number of arguments. Expected 2: path, eventName");
        }
        
        const std::string path = args[0].as_string();
        const std::string eventName = args[1].as_string();
        
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
                {"data", json_::Object({{"watcherId", watcherId}})}
            });
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });
    
    // 9. Read/Write Binary Data
    registerFunction("membrane_readBinaryFile", [this](const json &args) {
        if (args.size() != 1) {
            return retObj("error", "Invalid number of arguments. Expected 1: path");
        }
        
        const std::string path = args[0].as_string();
        
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
    
    registerFunction("membrane_writeBinaryFile", [this](const json &args) {
        if (args.size() != 2) {
            return retObj("error", "Invalid number of arguments. Expected 2: path, base64Data");
        }
        
        const std::string path = args[0].as_string();
        const std::string base64Data = args[1].as_string();
        
        try {
            // Decode base64 data
            std::vector<uint8_t> binaryData = base64Decode(base64Data);
            
            writeBinaryFile(path, binaryData);
            
            return retObj("success", "Binary file written successfully");
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });
    
    // 13. Temporary File Creation
    registerFunction("membrane_createTempFile", [this](const json &args) {
        std::string prefix = "membrane";
        std::string extension = ".tmp";
        
        if (args.size() >= 1) {
            prefix = args[0].as_string();
        }
        
        if (args.size() >= 2) {
            extension = args[1].as_string();
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
    // Group 1: External URL handling
    registerFunction("membrane_openExternalUrl", [this](const json &args) {
        if (args.size() != 1)
            return retObj("error", "Invalid number of arguments");
        const std::string url = args[0].as_string();
        try {
            openExternal(url);
            return retObj("success", "Opened external URL");
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });

    // Group 2: File operations
    registerFileSystemFunctions();

    // Group 3: VFS creation
    registerFunction("membrane_createCustomVfs", [this](const json &args) {
        if (args.size() != 1 && args.size() != 2)
            return retObj("error",
                          "Invalid number of arguments. Expected 1 or 2 "
                          "arguments: vfs_name, "
                          "[persistence_dir]");

        const std::string vfs_name = args[0].as_string();
        try {
            if (args.size() == 2) {
                const std::string persistence_dir = args[1].as_string();
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
    registerFunction("membrane_addFileToVfs", [this](const json &args) {
        if (args.size() != 3)
            return retObj("error",
                          "Invalid number of arguments. Expected 3 arguments: "
                          "vfs_name, path, content");

        const std::string vfs_name = args[0].as_string();
        const std::string path = args[1].as_string();
        const std::string content = args[2].as_string();

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
    registerFunction("membrane_saveVfsToDisk", [this](const json &args) {
        if (args.size() != 1) {
            return retObj(
                "error",
                "Invalid number of arguments. Expected 1 argument: vfs_name");
        }

        const std::string vfs_name = args[0].as_string();

        try {
            if (save_vfs_to_disk(vfs_name)) {
                return retObj("success", "Saved VFS to disk: " + vfs_name);
            }
            return retObj("error", "Failed to save VFS to disk: " + vfs_name);
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });

    registerFunction("membrane_saveAllVfsToDisk", [this](const json &) {
        try {
            if (save_all_vfs_to_disk()) {
                return retObj("success", "Saved all VFS instances to disk");
            }
            return retObj("error", "Failed to save some VFS instances to disk");
        } catch (const std::exception &e) {
            return retObj("error", e.what());
        }
    });

    registerFunction("membrane_writeClipboard", [](const json &args) {
        if (args.size() != 1 || !args[0].is_string()) {
            return retObj("error",
                          "Expected 1 string argument for clipboard content");
        }
        const std::string content = args[0].as_string();
        if (writeClipboard(content)) {
            return retObj("success", "Clipboard updated");
        } else {
            return retObj("error", "Failed to write to clipboard");
        }
    });

    registerFunction("membrane_readClipboard", [](const json &) {
        const std::string content = readClipboard();
        if (content.empty()) {
            return retObj("error",
                          "Failed to read clipboard or clipboard is empty");
        }
        return retObj("success", "Clipboard content", content);
    });

    registerFunction("membrane_listFunctions", [this](const json &) {
        std::vector<std::string>functions_list = _functionRegistry.getRegisteredFunctions();
        return json({
            {"status", "success"},
            {"message", "Registered functions"},
            {"data", functions_list},
        });
    });
}
