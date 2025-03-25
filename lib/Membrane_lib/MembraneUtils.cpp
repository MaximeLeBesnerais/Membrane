#include "MembraneUtils.hpp"

void openExternal(const std::string &url) {
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

void saveFile(const std::string &path, const std::string &content) {
    std::ofstream file(path, std::ios::out | std::ios::binary);
    file.write(content.c_str(), static_cast<long>(content.size()));
    file.close();
}

// Read file content (1)
std::string readFile(const std::string &path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Read content
    std::string content(size, ' ');
    file.read(&content[0], size);
    file.close();
    
    return content;
}

// Check file existence (2)
bool fileExists(const std::string &path) {
    std::ifstream file(path);
    return file.good();
}

// List directory contents (3)
std::vector<std::string> listDirectory(const std::string &path) {
    std::vector<std::string> result;
    
    try {
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
            result.push_back(entry.path().filename().string());
        }
    } catch (const std::exception &e) {
        throw std::runtime_error("Failed to list directory: " + std::string(e.what()));
    }
    
    return result;
}

// Create directory (4)
bool createDirectory(const std::string &path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (const std::exception &e) {
        throw std::runtime_error("Failed to create directory: " + std::string(e.what()));
    }
}

// Copy file/directory (5)
void copyFile(const std::string &source, const std::string &destination) {
    try {
        if (std::filesystem::is_directory(source)) {
            std::filesystem::copy(
                source, 
                destination, 
                std::filesystem::copy_options::recursive
            );
        } else {
            std::filesystem::copy_file(
                source, 
                destination, 
                std::filesystem::copy_options::overwrite_existing
            );
        }
    } catch (const std::exception &e) {
        throw std::runtime_error("Failed to copy: " + std::string(e.what()));
    }
}

// Delete file/directory (6)
bool deleteFileOrDirectory(const std::string &path) {
    try {
        return std::filesystem::remove_all(path) > 0;
    } catch (const std::exception &e) {
        throw std::runtime_error("Failed to delete: " + std::string(e.what()));
    }
}

// Get file information (7)
json getFileInfo(const std::string &path) {
    try {
        std::filesystem::path fsPath(path);
        std::filesystem::file_status status = std::filesystem::status(fsPath);
        auto lastModTime = std::filesystem::last_write_time(fsPath);
        auto timePoint = std::chrono::file_clock::to_sys(lastModTime);
        auto timeT = std::chrono::system_clock::to_time_t(timePoint);
        
        json info = {
            {"exists", std::filesystem::exists(status)},
            {"isRegularFile", std::filesystem::is_regular_file(status)},
            {"isDirectory", std::filesystem::is_directory(status)},
            {"isSymlink", std::filesystem::is_symlink(status)}
        };
        
        if (std::filesystem::exists(status)) {
            info["size"] = std::filesystem::file_size(fsPath);
            // Set the time string and remove trailing newline
            std::string lastMod = std::ctime(&timeT);
            if (!lastMod.empty() && lastMod[lastMod.length() - 1] == '\n') {
                lastMod.erase(lastMod.length() - 1);
            }
            info["lastModified"] = lastMod;
            
            // Get path components
            info["filename"] = fsPath.filename().string();
            info["extension"] = fsPath.extension().string();
            info["parent"] = fsPath.parent_path().string();
        }
        
        return info;
    } catch (const std::exception &e) {
        throw std::runtime_error("Failed to get file info: " + std::string(e.what()));
    }
}

// Watch file/directory for changes (8)
// Note: This is a simplified implementation that returns a watcher ID
// A real implementation would need to manage multiple watchers and callbacks
int watchFileOrDirectory(const std::string &path, std::function<void(const std::string&, const std::string&)> callback) {
    static int nextWatcherId = 1;
    static std::unordered_map<int, std::thread> watchers;
    static std::mutex watchersMutex;
    
    int watcherId = nextWatcherId++;
    
    std::thread watcherThread([path, callback, watcherId]() {
        try {
            auto lastWriteTime = std::filesystem::last_write_time(path);
            bool isDir = std::filesystem::is_directory(path);
            std::map<std::string, std::filesystem::file_time_type> dirEntryTimes;
            
            if (isDir) {
                for (const auto &entry : std::filesystem::directory_iterator(path)) {
                    dirEntryTimes[entry.path().string()] = std::filesystem::last_write_time(entry.path());
                }
            }
            
            while (true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                
                try {
                    if (isDir) {
                        // Check for new, modified or deleted files
                        std::map<std::string, std::filesystem::file_time_type> currentEntryTimes;
                        
                        for (const auto &entry : std::filesystem::directory_iterator(path)) {
                            std::string entryPath = entry.path().string();
                            auto currentTime = std::filesystem::last_write_time(entry.path());
                            currentEntryTimes[entryPath] = currentTime;
                            
                            if (dirEntryTimes.find(entryPath) == dirEntryTimes.end()) {
                                // New file
                                callback("created", entryPath);
                            } else if (currentTime != dirEntryTimes[entryPath]) {
                                // Modified file
                                callback("modified", entryPath);
                            }
                        }
                        
                        // Check for deleted files
                        for (const auto &[entryPath, time] : dirEntryTimes) {
                            if (currentEntryTimes.find(entryPath) == currentEntryTimes.end()) {
                                callback("deleted", entryPath);
                            }
                        }
                        
                        dirEntryTimes = currentEntryTimes;
                    } else {
                        // Single file watch
                        auto currentWriteTime = std::filesystem::last_write_time(path);
                        if (currentWriteTime != lastWriteTime) {
                            lastWriteTime = currentWriteTime;
                            callback("modified", path);
                        }
                    }
                } catch (const std::exception &e) {
                    // The file/directory may have been deleted
                    callback("error", std::string(e.what()));
                    break;
                }
            }
        } catch (const std::exception &e) {
            callback("error", std::string(e.what()));
        }
    });
    
    watcherThread.detach();
    
    {
        std::lock_guard<std::mutex> lock(watchersMutex);
        watchers[watcherId] = std::move(watcherThread);
    }
    
    return watcherId;
}

// Read/Write binary data (9)
std::vector<uint8_t> readBinaryFile(const std::string &path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Read content
    std::vector<uint8_t> content(size);
    file.read(reinterpret_cast<char*>(content.data()), size);
    file.close();
    
    return content;
}

void writeBinaryFile(const std::string &path, const std::vector<uint8_t> &data) {
    std::ofstream file(path, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + path);
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();
}

// Temporary file creation (13)
std::string createTempFile(const std::string &prefix, const std::string &extension) {
    std::string tempDir = std::filesystem::temp_directory_path().string();
    std::string tempFileName = prefix + "_" + 
        std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) +
        extension;
    std::string tempFilePath = tempDir + "/" + tempFileName;
    
    // Replace backslashes with forward slashes for consistent paths
    std::replace(tempFilePath.begin(), tempFilePath.end(), '\\', '/');
    
    // Create an empty file
    std::ofstream file(tempFilePath);
    file.close();
    
    if (!fileExists(tempFilePath)) {
        throw std::runtime_error("Failed to create temporary file");
    }
    
    return tempFilePath;
}

std::string get_app_data_directory(const std::string &app_name) {
    std::string dir;
#ifdef _WIN32
    char *appdata = nullptr;
    size_t len = 0;
    _dupenv_s(&appdata, &len, "APPDATA");
    if (appdata != nullptr) {
        dir = std::string(appdata) + "\\" + app_name;
        free(appdata);
    }
#elif defined(__APPLE__)
    dir = std::string(getenv("HOME")) + "/Library/Application Support/" +
          app_name;
#else  // Linux and others
    dir = std::string(getenv("HOME")) + "/.local/share/" + app_name;
#endif
    return dir;
}

bool writeClipboard(const std::string &text) {
#ifdef _WIN32
    if (!OpenClipboard(nullptr)) return false;
    EmptyClipboard();
    HGLOBAL hGlob = GlobalAlloc(GMEM_FIXED, text.size() + 1);
    if (!hGlob) return false;
    memcpy(hGlob, text.c_str(), text.size() + 1);
    SetClipboardData(CF_TEXT, hGlob);
    CloseClipboard();
    GlobalFree(hGlob);
    return true;
#elif defined(__APPLE__)
    std::string cmd = "echo " + text + " | pbcopy";
    return system(cmd.c_str()) == 0;
#else
    std::string cmd = "echo '" + text + "' | xclip -selection clipboard";
    return system(cmd.c_str()) == 0;
#endif
}

std::string readClipboard() {
#ifdef _WIN32
    if (!OpenClipboard(nullptr)) return "";
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr) return "";
    char *pszText = static_cast<char *>(GlobalLock(hData));
    std::string text = pszText ? std::string(pszText) : "";
    GlobalUnlock(hData);
    CloseClipboard();
    return text;
#elif defined(__APPLE__)
    char buffer[1024];
    FILE *pipe = popen("pbpaste", "r");
    if (!pipe) return "";
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
#else
    char buffer[1024];
    FILE *pipe = popen("xclip -selection clipboard -o", "r");
    if (!pipe) return "";
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
#endif
}
