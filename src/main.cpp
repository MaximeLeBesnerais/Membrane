#include "Membrane_lib/Membrane.hpp"
#include "resource_init.hpp"
#include <libssh/libssh.h>
#include <fstream>
#include <filesystem>
#include <random>
#include <chrono>

// Helper function to generate an SSH key pair
json generateSSHKey(const std::string& keyType, 
                   const std::string& keySize, 
                   const std::string& email, 
                   const std::string& comment, 
                   const std::string& passphrase) {
    
    ssh_key key = nullptr;
    int rc;
    enum ssh_keytypes_e key_type;
    int bits;
    
    // Convert keyType to libssh key type
    if (keyType == "RSA") {
        key_type = SSH_KEYTYPE_RSA;
        bits = std::stoi(keySize);
    } else if (keyType == "ED25519") {
        key_type = SSH_KEYTYPE_ED25519;
        bits = 256; // ED25519 is always 256 bits
    } else if (keyType == "ECDSA") {
        key_type = SSH_KEYTYPE_ECDSA;
        bits = 256; // ECDSA is 256 bits for our use case
    } else if (keyType == "DSA") {
        key_type = SSH_KEYTYPE_DSS;
        bits = 1024; // DSA is typically 1024 bits
    } else {
        return json({
            {"status", "error"},
            {"message", "Invalid key type"},
            {"publicKey", ""},
            {"privateKey", ""}
        });
    }
    
    // Generate the key
    rc = ssh_pki_generate(key_type, bits, &key);
    if (rc != SSH_OK) {
        return json({
            {"status", "error"},
            {"message", "Failed to generate key"},
            {"publicKey", ""},
            {"privateKey", ""}
        });
    }
    
    std::string app_dir = get_app_data_directory("SSH Key Gen");
    std::filesystem::create_directories(app_dir);
    
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()).count();
    
    std::string sanitized_email = email;
    std::ranges::replace(sanitized_email, '@', '_');
    std::ranges::replace(sanitized_email, '.', '_');
    
    std::string filename_base = app_dir + "/" + sanitized_email + "_" + 
                               std::to_string(timestamp);
    
    std::string key_comment = email;
    if (!comment.empty()) {
        key_comment += " " + comment;
    }
    
    char* public_key_str = nullptr;
    rc = ssh_pki_export_pubkey_base64(key, &public_key_str);
    if (rc != SSH_OK || public_key_str == nullptr) {
        ssh_key_free(key);
        return json({
            {"status", "error"},
            {"message", "Failed to export public key"},
            {"publicKey", ""},
            {"privateKey", ""}
        });
    }
    
    std::string key_type_str;
    if (keyType == "RSA") {
        key_type_str = "ssh-rsa";
    } else if (keyType == "ED25519") {
        key_type_str = "ssh-ed25519";
    } else if (keyType == "ECDSA") {
        key_type_str = "ecdsa-sha2-nistp256";
    } else { // DSA
        key_type_str = "ssh-dss";
    }
    
    std::string public_key = key_type_str + " " + public_key_str + " " + key_comment;
    
    std::string public_key_file = filename_base + ".pub";
    std::ofstream pub_file(public_key_file);
    pub_file << public_key << std::endl;
    pub_file.close();
    
    char* private_key_str = nullptr;
    rc = ssh_pki_export_privkey_base64(key, 
                                      passphrase.empty() ? nullptr : passphrase.c_str(), 
                                      nullptr, nullptr, &private_key_str);
    
    if (rc != SSH_OK || private_key_str == nullptr) {
        ssh_key_free(key);
        ssh_string_free_char(public_key_str);
        return json({
            {"status", "error"},
            {"message", "Failed to export private key"},
            {"publicKey", public_key},
            {"privateKey", ""}
        });
    }
    
    std::string private_key_file = filename_base;
    std::ofstream priv_file(private_key_file);
    priv_file << private_key_str << std::endl;
    priv_file.close();
    
    // Free memory
    ssh_string_free_char(public_key_str);
    ssh_string_free_char(private_key_str);
    ssh_key_free(key);
    
    return json({
        {"status", "success"},
        {"message", "Key generated successfully"},
        {"publicKey", public_key},
        {"privateKey", private_key_file} 
    });
}

int main() {
    Membrane app("SSH Key Gen", "index.html", 620, 880, WEBVIEW_HINT_FIXED);
    initialize_resources(app);
    
    // Register the generateSSHKey function to be callable from JavaScript
    app.registerFunction("generateSSHKey", [](const json& args) {
        if (args.size() != 5) {
            return json({
                {"status", "error"},
                {"message", "Invalid arguments"},
                {"publicKey", ""},
                {"privateKey", ""}
            });
        }
        
        try {
            std::string keyType = args[0].get<std::string>();
            std::string keySize = args[1].get<std::string>();
            std::string email = args[2].get<std::string>();
            std::string comment = args[3].get<std::string>();
            std::string passphrase = args[4].get<std::string>();
            
            return generateSSHKey(keyType, keySize, email, comment, passphrase);
        } 
        catch (const std::exception& e) {
            return json({
                {"status", "error"},
                {"message", std::string("Error: ") + e.what()},
                {"publicKey", ""},
                {"privateKey", ""}
            });
        }
    });

    app.run();
    return 0;
}