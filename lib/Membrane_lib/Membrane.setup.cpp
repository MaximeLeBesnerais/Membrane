#include "Membrane.hpp"

json retObj(std::string status, std::string message, const std::string &data) {
    return json({{"status", status}, {"message", message}, {"data", data}});
}

void Membrane::setTools() {
    // Group 1: External URL handling
    registerFunction("membrane_openExternalUrl", [this](const json &args) {
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
    registerFunction("membrane_saveFile", [this](const json &args) {
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

    // Group 3: VFS creation
    registerFunction("membrane_createCustomVfs", [this](const json &args) {
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
    registerFunction("membrane_addFileToVfs", [this](const json &args) {
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
    registerFunction("membrane_saveVfsToDisk", [this](const json &args) {
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
}
