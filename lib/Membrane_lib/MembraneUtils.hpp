// Membrane - A C++ and Web Tech Interface
// Created by Maxime Le Besnerais
// Copyright (c) 2025 Maxime Le Besnerais

#ifndef MEMBRANE_UTILS_HPP
#define MEMBRANE_UTILS_HPP
#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include <thread>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 1. External URL handling
void openExternal(const std::string &url);

// 2. File operations
void saveFile(const std::string &path, const std::string &content);

// 3. Helper functions
std::string get_app_data_directory(const std::string &app_name);

// 4. Clipboard operations
bool writeClipboard(const std::string &text);
std::string readClipboard();
// File content operations
std::string readFile(const std::string &path);
void saveFile(const std::string &path, const std::string &content);

// File existence and info
bool fileExists(const std::string &path);
json getFileInfo(const std::string &path);

// Directory operations
std::vector<std::string> listDirectory(const std::string &path);
bool createDirectory(const std::string &path);

// File operations
void copyFile(const std::string &source, const std::string &destination);
bool deleteFileOrDirectory(const std::string &path);

// File watching
int watchFileOrDirectory(const std::string &path, std::function<void(const std::string&, const std::string&)> callback);

// Binary data
std::vector<uint8_t> readBinaryFile(const std::string &path);
void writeBinaryFile(const std::string &path, const std::vector<uint8_t> &data);

// Temporary files
std::string createTempFile(const std::string &prefix, const std::string &extension);

// Helper functions
std::string base64Encode(const uint8_t* data, size_t length);
std::vector<uint8_t> base64Decode(const std::string& encoded);

// Registration function
void registerFileSystemFunctions();

#endif /* MEMBRANE_UTILS_HPP */
