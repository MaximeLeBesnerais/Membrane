#include <gtest/gtest.h>
#include "FunctionRegistry.hpp"
#include <stdexcept>
#include <string>

class FunctionRegistryTest : public ::testing::Test {
protected:
    FunctionRegistry registry;
    
    void SetUp() override {
        // Register some test functions
        registry.registerFunction("add", [](const json& args) -> json {
            if (!args.contains("a") || !args.contains("b")) {
                throw std::invalid_argument("Missing required parameters 'a' or 'b'");
            }
            
            int a = args["a"];
            int b = args["b"];
            return {{"status", "success"}, {"data", a + b}};
        });
        
        registry.registerFunction("multiply", [](const json& args) -> json {
            if (!args.contains("a") || !args.contains("b")) {
                throw std::invalid_argument("Missing required parameters 'a' or 'b'");
            }
            
            int a = args["a"];
            int b = args["b"];
            return {{"status", "success"}, {"data", a * b}};
        });
        
        registry.registerFunction("greet", [](const json& args) -> json {
            std::string name = args.value("name", "Guest");
            return {{"status", "success"}, {"data", "Hello, " + name + "!"}};
        });
    }
};

TEST_F(FunctionRegistryTest, RegisterAndHasFunction) {
    EXPECT_TRUE(registry.hasFunction("add"));
    EXPECT_TRUE(registry.hasFunction("multiply"));
    EXPECT_TRUE(registry.hasFunction("greet"));
    EXPECT_FALSE(registry.hasFunction("nonexistent"));
}

TEST_F(FunctionRegistryTest, CallExistingFunction) {
    // Test add function
    json add_result = registry.callFunction("add", {{"a", 5}, {"b", 3}});
    EXPECT_EQ(add_result["status"], "success");
    EXPECT_EQ(add_result["data"], 8);
    
    // Test multiply function
    json multiply_result = registry.callFunction("multiply", {{"a", 4}, {"b", 7}});
    EXPECT_EQ(multiply_result["status"], "success");
    EXPECT_EQ(multiply_result["data"], 28);
    
    // Test greet function with parameter
    json greet_result = registry.callFunction("greet", {{"name", "John"}});
    EXPECT_EQ(greet_result["status"], "success");
    EXPECT_EQ(greet_result["data"], "Hello, John!");
    
    // Test greet function with default parameter
    json greet_default_result = registry.callFunction("greet", json::object());
    EXPECT_EQ(greet_default_result["status"], "success");
    EXPECT_EQ(greet_default_result["data"], "Hello, Guest!");
}

TEST_F(FunctionRegistryTest, CallNonexistentFunction) {
    json result = registry.callFunction("nonexistent", {{"param", "value"}});
    EXPECT_EQ(result["status"], "error");
    EXPECT_EQ(result["message"], "Function not found: nonexistent");
    EXPECT_EQ(result["data"], nullptr);
}

TEST_F(FunctionRegistryTest, CallFunctionWithInvalidArguments) {
    // Missing required parameters
    json add_result = registry.callFunction("add", {{"a", 5}});
    EXPECT_EQ(add_result["status"], "error");
    EXPECT_EQ(add_result["message"], "Missing required parameters 'a' or 'b'");
    EXPECT_EQ(add_result["data"], nullptr);
}

TEST_F(FunctionRegistryTest, RegisterNewFunction) {
    // Register a new function
    registry.registerFunction("subtract", [](const json& args) -> json {
        int a = args["a"];
        int b = args["b"];
        return {{"status", "success"}, {"data", a - b}};
    });
    
    EXPECT_TRUE(registry.hasFunction("subtract"));
    
    // Test the newly registered function
    json result = registry.callFunction("subtract", {{"a", 10}, {"b", 4}});
    EXPECT_EQ(result["status"], "success");
    EXPECT_EQ(result["data"], 6);
}

TEST_F(FunctionRegistryTest, OverrideExistingFunction) {
    // Override the add function
    registry.registerFunction("add", [](const json& args) -> json {
        int a = args["a"];
        int b = args["b"];
        return {{"status", "success"}, {"data", a + b + 1}}; // Add 1 to the result
    });
    
    // Test the overridden function
    json result = registry.callFunction("add", {{"a", 5}, {"b", 3}});
    EXPECT_EQ(result["status"], "success");
    EXPECT_EQ(result["data"], 9); // 5 + 3 + 1 = 9
}

TEST_F(FunctionRegistryTest, ComplexJsonArguments) {
    // Register a function that works with complex JSON
    registry.registerFunction("processComplex", [](const json& args) -> json {
        json result = {{"status", "success"}, {"processed", json::array()}};
        
        if (args.contains("items") && args["items"].is_array()) {
            for (const auto& item : args["items"]) {
                if (item.contains("value")) {
                    result["processed"].push_back({
                        {"original", item["value"]},
                        {"doubled", item["value"].get<int>() * 2}
                    });
                }
            }
        }
        
        return result;
    });
    
    // Test with complex JSON
    json complex_args = {
        {"items", json::array({
            {{"id", 1}, {"value", 10}},
            {{"id", 2}, {"value", 20}},
            {{"id", 3}, {"value", 30}}
        })}
    };
    
    json result = registry.callFunction("processComplex", complex_args);
    EXPECT_EQ(result["status"], "success");
    EXPECT_EQ(result["processed"].size(), 3);
    EXPECT_EQ(result["processed"][0]["original"], 10);
    EXPECT_EQ(result["processed"][0]["doubled"], 20);
    EXPECT_EQ(result["processed"][1]["original"], 20);
    EXPECT_EQ(result["processed"][1]["doubled"], 40);
    EXPECT_EQ(result["processed"][2]["original"], 30);
    EXPECT_EQ(result["processed"][2]["doubled"], 60);
}

TEST_F(FunctionRegistryTest, FunctionThrowingException) {
    // Register a function that intentionally throws an exception
    registry.registerFunction("throwsException", [](const json& args) -> json {
        throw std::runtime_error("Intentional test exception");
    });
    
    // Test the function that throws an exception
    json result = registry.callFunction("throwsException", {});
    EXPECT_EQ(result["status"], "error");
    EXPECT_EQ(result["message"], "Intentional test exception");
    EXPECT_EQ(result["data"], nullptr);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}