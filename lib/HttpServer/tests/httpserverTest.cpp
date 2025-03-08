#include <gtest/gtest.h>
#include "HttpServer.hpp"
#include <thread>
#include <future>
#include <curl/curl.h>
#include <memory>
#include <sstream>
#include <fstream>

// Helper class for creating a mock VirtualFileSystem
class MockVFS : public VirtualFileSystem {
public:
    MockVFS() = default;

    void add_file(const std::string& path, const std::string& content, const std::string& mime_type) {
        VirtualFileSystem::FileEntry entry;
        entry.data = std::vector<uint8_t>(content.begin(), content.end());
        entry.mime_type = mime_type;
        files[path] = entry;
    }

    bool exists(const std::string& path) const override {
        return files.find(path) != files.end();
    }

    const FileEntry* get_file(const std::string& path) const override {
        auto it = files.find(path);
        if (it != files.end()) {
            return &it->second;
        }
        return nullptr;
    }

private:
    std::unordered_map<std::string, FileEntry> files;
};

// Callback for CURL to write received data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append(static_cast<char*>(contents), total_size);
    return total_size;
}

class HttpServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    void TearDown() override {
        curl_global_cleanup();
    }

    // Helper method to make HTTP requests
    std::pair<long, std::string> make_request(const std::string& url, 
                                             const std::string& method = "GET", 
                                             const std::string& data = "",
                                             const std::vector<std::string>& headers = {}) {
        CURL* curl = curl_easy_init();
        std::string response_body;
        long http_code = 0;

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            
            // Set method
            if (method == "POST") {
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                if (!data.empty()) {
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
                }
            } else if (method != "GET") {
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
                if (!data.empty()) {
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
                }
            }

            // Set headers if provided
            struct curl_slist* headerlist = nullptr;
            for (const auto& header : headers) {
                headerlist = curl_slist_append(headerlist, header.c_str());
            }
            if (headerlist) {
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
            }

            // Set write callback
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
            
            // Execute the request
            CURLcode res = curl_easy_perform(curl);
            
            // Check for errors
            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            }
            
            // Get HTTP response code
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            
            // Clean up
            if (headerlist) {
                curl_slist_free_all(headerlist);
            }
            curl_easy_cleanup(curl);
        }
        
        return {http_code, response_body};
    }
};

TEST_F(HttpServerTest, ServerStartsAndStops) {
    HttpServer server(8080);
    ASSERT_TRUE(server.start());
    EXPECT_TRUE(server.is_running());
    server.stop();
    EXPECT_FALSE(server.is_running());
}

TEST_F(HttpServerTest, ServerHandlesRoutes) {
    HttpServer server(8081);
    
    // Register a simple route
    server.register_route("/test", [](
        const std::string& method,
        const std::unordered_map<std::string, std::string>& headers,
        const std::string& body,
        std::string& response_body,
        std::unordered_map<std::string, std::string>& response_headers) {
            response_body = "Test successful";
            response_headers["Content-Type"] = "text/plain";
    });
    
    ASSERT_TRUE(server.start());
    
    // Run in a separate thread to allow testing
    auto future = std::async(std::launch::async, [this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Give server time to start
        auto [status, body] = make_request("http://localhost:8081/test");
        return std::make_pair(status, body);
    });
    
    auto [status, body] = future.get();
    EXPECT_EQ(status, 200);
    EXPECT_EQ(body, "Test successful");
    
    server.stop();
}

TEST_F(HttpServerTest, ServerHandlesNotFound) {
    HttpServer server(8082);
    ASSERT_TRUE(server.start());
    
    auto future = std::async(std::launch::async, [this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto [status, body] = make_request("http://localhost:8082/nonexistent");
        return status;
    });
    
    long status = future.get();
    EXPECT_EQ(status, 404);
    
    server.stop();
}

TEST_F(HttpServerTest, ServerServesVFSFiles) {
    HttpServer server(8083);
    auto mock_vfs = std::make_unique<MockVFS>();
    
    // Add a file to the mock VFS
    mock_vfs->add_file("index.html", "<html><body>Hello from VFS</body></html>", "text/html");
    mock_vfs->add_file("test.css", "body { color: red; }", "text/css");
    
    // Mount the VFS at a specific path
    server.mount_vfs("/static", mock_vfs.get());
    
    ASSERT_TRUE(server.start());
    
    // Test index.html
    auto future1 = std::async(std::launch::async, [this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto [status, body] = make_request("http://localhost:8083/static/index.html");
        return std::make_pair(status, body);
    });
    
    auto [status1, body1] = future1.get();
    EXPECT_EQ(status1, 200);
    EXPECT_EQ(body1, "<html><body>Hello from VFS</body></html>");
    
    // Test test.css
    auto future2 = std::async(std::launch::async, [this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto [status, body] = make_request("http://localhost:8083/static/test.css");
        return std::make_pair(status, body);
    });
    
    auto [status2, body2] = future2.get();
    EXPECT_EQ(status2, 200);
    EXPECT_EQ(body2, "body { color: red; }");
    
    server.stop();
}

TEST_F(HttpServerTest, ServerHandlesPostRequest) {
    HttpServer server(8084);
    
    // Register a route that handles POST
    server.register_route("/api/data", [](
        const std::string& method,
        const std::unordered_map<std::string, std::string>& headers,
        const std::string& body,
        std::string& response_body,
        std::unordered_map<std::string, std::string>& response_headers) {
            if (method == "POST") {
                response_body = "Received: " + body;
                response_headers["Content-Type"] = "text/plain";
            } else {
                response_body = "Method not allowed";
                response_headers["Content-Type"] = "text/plain";
            }
    });
    
    ASSERT_TRUE(server.start());
    
    auto future = std::async(std::launch::async, [this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::string data = "Hello, server!";
        std::vector<std::string> headers = {"Content-Type: text/plain"};
        auto [status, body] = make_request("http://localhost:8084/api/data", "POST", data, headers);
        return std::make_pair(status, body);
    });
    
    auto [status, body] = future.get();
    EXPECT_EQ(status, 200);
    EXPECT_EQ(body, "Received: Hello, server!");
    
    server.stop();
}

TEST_F(HttpServerTest, ServerHandlesHeaders) {
    HttpServer server(8085);
    
    // Register a route that echoes a specific header
    server.register_route("/echo-header", [](
        const std::string& method,
        const std::unordered_map<std::string, std::string>& headers,
        const std::string& body,
        std::string& response_body,
        std::unordered_map<std::string, std::string>& response_headers) {
            auto it = headers.find("x-custom-header");
            if (it != headers.end()) {
                response_body = "Header value: " + it->second;
            } else {
                response_body = "Header not found";
            }
            response_headers["Content-Type"] = "text/plain";
    });
    
    ASSERT_TRUE(server.start());
    
    auto future = std::async(std::launch::async, [this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::vector<std::string> headers = {"X-Custom-Header: test-value"};
        auto [status, body] = make_request("http://localhost:8085/echo-header", "GET", "", headers);
        return std::make_pair(status, body);
    });
    
    auto [status, body] = future.get();
    EXPECT_EQ(status, 200);
    EXPECT_EQ(body, "Header value: test-value");
    
    server.stop();
}

TEST_F(HttpServerTest, ServerHandlesDefaultIndex) {
    HttpServer server(8086);
    auto mock_vfs = std::make_unique<MockVFS>();
    
    // Add an index.html file
    mock_vfs->add_file("index.html", "<html><body>Default Index</body></html>", "text/html");
    
    // Mount the VFS
    server.mount_vfs("/", mock_vfs.get());
    
    ASSERT_TRUE(server.start());
    
    auto future = std::async(std::launch::async, [this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto [status, body] = make_request("http://localhost:8086/");
        return std::make_pair(status, body);
    });
    
    auto [status, body] = future.get();
    EXPECT_EQ(status, 200);
    EXPECT_EQ(body, "<html><body>Default Index</body></html>");
    
    server.stop();
}

TEST_F(HttpServerTest, ServerHandlesMultipleRequests) {
    HttpServer server(8087);
    
    // Register a simple route
    server.register_route("/count", [count = 0](
        const std::string& method,
        const std::unordered_map<std::string, std::string>& headers,
        const std::string& body,
        std::string& response_body,
        std::unordered_map<std::string, std::string>& response_headers) mutable {
            response_body = "Request count: " + std::to_string(++count);
            response_headers["Content-Type"] = "text/plain";
    });
    
    ASSERT_TRUE(server.start());
    
    auto make_count_request = [this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto [status, body] = make_request("http://localhost:8087/count");
        return std::make_pair(status, body);
    };
    
    // Make 3 sequential requests
    auto future1 = std::async(std::launch::async, make_count_request);
    auto [status1, body1] = future1.get();
    EXPECT_EQ(status1, 200);
    EXPECT_EQ(body1, "Request count: 1");
    
    auto future2 = std::async(std::launch::async, make_count_request);
    auto [status2, body2] = future2.get();
    EXPECT_EQ(status2, 200);
    EXPECT_EQ(body2, "Request count: 2");
    
    auto future3 = std::async(std::launch::async, make_count_request);
    auto [status3, body3] = future3.get();
    EXPECT_EQ(status3, 200);
    EXPECT_EQ(body3, "Request count: 3");
    
    server.stop();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
