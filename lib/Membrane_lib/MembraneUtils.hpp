// Membrane - A C++ and Web Tech Interface
// Created by Maxime Le Besnerais
// Copyright (c) 2025 Maxime Le Besnerais

#ifndef MEMBRANE_UTILS_HPP
#define MEMBRANE_UTILS_HPP
#include <string>
#include <fstream>
#include <iostream>
// 1. External URL handling
void openExternal(const std::string &url);

// 2. File operations
void saveFile(const std::string &path, const std::string &content);

// 3. Helper functions
std::string get_app_data_directory(const std::string &app_name);
#endif /* MEMBRANE_UTILS_HPP */
