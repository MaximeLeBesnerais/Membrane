// FunctionRegistry.cpp
#include "FunctionRegistry.hpp"
#include <stdexcept>

void FunctionRegistry::registerFunction(const std::string& name, RegisteredFunction func) {
    if (!functions.insert({name, std::move(func)}).second && !_canOverwrite) {
        std::string message = "By default, functions cannot be overwritten.";
        message += "To allow overwriting, call canOverwrite(true) first.";
        throw std::runtime_error(message);
    }
    functions[name] = std::move(func);
}

json FunctionRegistry::callFunction(const std::string& name, const json& args) {
    const auto it = functions.find(name);
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
    return functions.contains(name);
}
