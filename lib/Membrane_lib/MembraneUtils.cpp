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
