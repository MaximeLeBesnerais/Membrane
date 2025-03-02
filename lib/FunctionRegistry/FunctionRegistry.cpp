// FunctionRegistry.cpp
#include "FunctionRegistry.hpp"
#include <stdexcept>

void FunctionRegistry::registerFunction(const std::string& name, RegisteredFunction func) {
    functions[name] = std::move(func);
}

json FunctionRegistry::callFunction(const std::string& name, const json& args) {
    auto it = functions.find(name);
    if (it == functions.end()) {
        return {
                {"status", "error"},
                {"message", "Function not found: " + name},
                {"data", nullptr}
        };
    }

    try {
        return it->second(args);
    } catch (const std::exception& e) {
        return {
                {"status", "error"},
                {"message", e.what()},
                {"data", nullptr}
        };
    }
}

bool FunctionRegistry::hasFunction(const std::string& name) const {
    return functions.find(name) != functions.end();
}
