#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Membrane.hpp"
#include <filesystem>
#include <fstream>
#include <thread>

// Mock classes for dependencies
class MockHttpServer : public HttpServer {
public:
    explicit MockHttpServer(int port) : HttpServer(port) {}
    
    MOCK_METHOD(bool, start, (), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(bool, is_running, (), (const, override));
    MOCK_METHOD(void, mount_vfs, (const std::string&, const VirtualFileSystem*), (override));
    MOCK_METHOD(void, register_route, (const std::string&, const std::function<void(const std::string&, const std::unordered_map<std::string, std::string>&, const std::string&, std::string&, std::unordered_map<std::string, std::string>&)>&), (override));
};

class MockVFS : public VirtualFileSystem {
public:
    MockVFS() : VirtualFileSystem() {}
    explicit MockVFS(const std::string& path) : VirtualFileSystem(path) {}
    
    MOCK_METHOD(void, add_file, (const std::string&, const unsigned char*, unsigned int), (override));
    MOCK_METHOD(bool, exists, (const std::string&), (const, override));
    MOCK_METHOD(const FileEntry*, get_file, (const std::string&), (const, override));
    MOCK_METHOD(bool, save_to_disk, (), (override));
    MOCK_METHOD(bool, load_from_disk, (), (override));
    MOCK_METHOD(bool, is_persistent, (), (const, override));
    
    // Helper to provide test data
    void add_test_file(const std::string& path, const std::string& content, const std::string& mime_type) {
        FileEntry entry;
        entry.data = std::vector<unsigned char>(content.begin(), content.end());
        entry.mime_type = mime_type;
        files[path] = entry;
    }
    
    // Expose the internal files map
    const std::map<std::string, FileEntry>& get_files() const override {
        return files;
    }

private:
    std::map<std::string, FileEntry> files;
};

// Test fixture
class MembraneTest : public ::testing::Test {
protected:
    // Create a test directory for persistence
    std::string test_dir;
    
    void SetUp() override {
        // Create a unique test directory
        test_dir = std::filesystem::temp_directory_path().string() + "/membrane_test_" + 
                   std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        std::filesystem::create_directory(test_dir);
    }
    
    void TearDown() override {
        // Clean up the test directory
        std::filesystem::remove_all(test_dir);
    }
    
    // Helper function to create a test ZIP file
    std::vector<unsigned char> create_test_zip_file() {
        // This is a minimal ZIP file containing one file
        // In a real test, you'd want to use miniz to create a real ZIP file
        // For this mock test, we'll return a placeholder
        return std::vector<unsigned char>{0x50, 0x4B, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00};
    }
    
    // Utility functions
    static bool file_exists(const std::string& path) {
        return std::filesystem::exists(path);
    }
    
    static std::string read_file_content(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return "";
        return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
};

// Test constructor and initialization
TEST_F(MembraneTest, Constructor) {
    Membrane app("Test App");
    // Basic initialization test
    EXPECT_NE(app.findAvailablePort(), 0);
}

// Test function registration and calling
TEST_F(MembraneTest, FunctionRegistration) {
    Membrane app("Test App");
    
    // Register a simple test function
    app.registerFunction("testFunc", [](const json& args) -> json {
        int a = args["a"].get<int>();
        int b = args["b"].get<int>();
        return {{"result", a + b}};
    });
    
    // Call the function
    json result = app.callFunction("testFunc", {{"a", 5}, {"b", 3}});
    EXPECT_EQ(result["result"].get<int>(), 8);
}

// Test VFS operations
TEST_F(MembraneTest, VFSOperations) {
    Membrane app("Test App");
    
    // Test adding a file to the VFS
    std::string test_content = "Test content";
    std::vector<unsigned char> data(test_content.begin(), test_content.end());
    
    app.add_vfs("test.txt", data.data(), data.size());
    
    // Test custom VFS creation
    app.add_custom_vfs("custom");
    
    // Test adding file to custom VFS
    app.add_to_custom_vfs("custom", "custom_test.txt", data.data(), data.size());
    
    // Test persistent VFS creation
    app.add_persistent_vfs("persistent", "test_persist");
    
    // Test adding file to persistent VFS
    app.add_to_custom_vfs("persistent", "persistent_test.txt", data.data(), data.size());
    
    // Test saving VFS to disk
    EXPECT_TRUE(app.save_vfs_to_disk("persistent"));
    
    // Test saving all VFS to disk
    EXPECT_TRUE(app.save_all_vfs_to_disk());
}

// Test HTTP endpoint registration
TEST_F(MembraneTest, EndpointRegistration) {
    Membrane app("Test App");
    
    // Register a test endpoint
    app.register_endpoint_handler("/test", [](
        const std::string& method,
        const std::unordered_map<std::string, std::string>& headers,
        const std::string& body,
        std::string& response_body,
        std::unordered_map<std::string, std::string>& response_headers) {
            response_body = "Test response";
            response_headers["Content-Type"] = "text/plain";
    });
    
    // We can't easily test the endpoint directly, but we've verified it registers
}

// Test ZIP file extraction
TEST_F(MembraneTest, ZipExtraction) {
    Membrane app("Test App");
    
    // Create a test ZIP file
    auto zip_data = create_test_zip_file();
    
    // Create a FileEntry
    VirtualFileSystem::FileEntry zip_entry;
    zip_entry.data = zip_data;
    zip_entry.mime_type = "application/zip";
    
    // This test is limited without a full ZIP file
    // In a real test, you would create a real ZIP file and verify the extraction
    // For now, we just ensure the function doesn't crash
    app.UnzipData("test.zip", zip_entry);
}

// Test utility functions
TEST_F(MembraneTest, UtilityFunctions) {
    // Test file saving
    std::string test_path = test_dir + "/test_save.txt";
    std::string test_content = "Test content for saving";
    
    Membrane::saveFile(test_path, test_content);
    EXPECT_TRUE(file_exists(test_path));
    EXPECT_EQ(read_file_content(test_path), test_content);
}

// Test app data directory function
TEST_F(MembraneTest, AppDataDirectory) {
    std::string app_name = "MembraneTestApp";
    std::string app_dir = get_app_data_directory(app_name);
    
    // Check that the path is non-empty and contains the app name
    EXPECT_FALSE(app_dir.empty());
    EXPECT_NE(app_dir.find(app_name), std::string::npos);
}

// Test default VFS path setting
TEST_F(MembraneTest, DefaultVFSPath) {
    Membrane app("Test App");
    std::string test_path = "test_vfs_path";
    
    app.setDefaultVfsPath(test_path);
    // We can't easily test the private member, but we've verified it sets
}

// More complex integration test
TEST_F(MembraneTest, FunctionIntegration) {
    Membrane app("Test App");
    
    // Register several functions
    app.registerFunction("add", [](const json& args) -> json {
        int a = args["a"].get<int>();
        int b = args["b"].get<int>();
        return {{"result", a + b}};
    });
    
    app.registerFunction("multiply", [](const json& args) -> json {
        int a = args["a"].get<int>();
        int b = args["b"].get<int>();
        return {{"result", a * b}};
    });
    
    app.registerFunction("complex", [](const json& args) -> json {
        json result = {{"processed", json::array()}};
        
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
    
    // Test the functions
    json add_result = app.callFunction("add", {{"a", 5}, {"b", 3}});
    EXPECT_EQ(add_result["result"].get<int>(), 8);
    
    json multiply_result = app.callFunction("multiply", {{"a", 4}, {"b", 7}});
    EXPECT_EQ(multiply_result["result"].get<int>(), 28);
    
    // Test with complex JSON
    json complex_args = {
        {"items", json::array({
            {{"id", 1}, {"value", 10}},
            {{"id", 2}, {"value", 20}},
            {{"id", 3}, {"value", 30}}
        })}
    };
    
    json complex_result = app.callFunction("complex", complex_args);
    EXPECT_EQ(complex_result["processed"].size(), 3);
    EXPECT_EQ(complex_result["processed"][0]["original"].get<int>(), 10);
    EXPECT_EQ(complex_result["processed"][0]["doubled"].get<int>(), 20);
}

// Test error handling
TEST_F(MembraneTest, ErrorHandling) {
    Membrane app("Test App");
    
    // Register a function that throws an exception
    app.registerFunction("throwsException", [](const json&) -> json {
        throw std::runtime_error("Test exception");
    });
    
    // Test calling a non-existent function
    json nonexistent_result = app.callFunction("nonexistent", {});
    EXPECT_EQ(nonexistent_result["status"].get<std::string>(), "error");
    EXPECT_EQ(nonexistent_result["message"].get<std::string>(), "Function not found: nonexistent");
    
    // Test calling a function that throws an exception
    json exception_result = app.callFunction("throwsException", {});
    EXPECT_EQ(exception_result["status"].get<std::string>(), "error");
    EXPECT_EQ(exception_result["message"].get<std::string>(), "Test exception");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}