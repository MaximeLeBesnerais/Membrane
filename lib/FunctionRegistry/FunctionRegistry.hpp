// Membrane - A C++ and Web Tech Interface
// Created by Maxime Le Besnerais
// Copyright (c) 2025 Maxime Le Besnerais

#ifndef FUNCTIONREGISTRY_HPP
#define FUNCTIONREGISTRY_HPP
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

class FunctionRegistry {
public:
    using RegisteredFunction = std::function<json(const json &args)>;
    // Register a function with a name
    void registerFunction(const std::string &name, RegisteredFunction func);
    // Call a registered function by name with json arguments
    json callFunction(const std::string &name, const json &args);
    // Check if a function is registered
    bool hasFunction(const std::string &name) const;

private:
    std::unordered_map<std::string, RegisteredFunction> functions;
};
#endif  // FUNCTIONREGISTRY_HPP
