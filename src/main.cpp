#include "Membrane.hpp"
#include "about.hpp"
#include "index.hpp"
#include <iostream>

int main() {
    // Create a Membrane instance with debug enabled
    Membrane app(true);

    // Set application metadata
    app.setTitle("Membrane VFS Demo");
    app.setSize(1024, 768);

    // Add static resources
    const std::string about_html_str(reinterpret_cast<const char*>(res_about_html), res_about_html_len);
    const std::string index_html_str(reinterpret_cast<const char*>(res_index_html), res_index_html_len);

    app.addResource("membrane://index.html", index_html_str);
    app.addResource("membrane://about.html", about_html_str);

    // Add a custom API method
    app.registerAPI("getCurrentTime", [](const std::string&) -> std::string {
        const auto now = std::chrono::system_clock::now();
        const auto time_t = std::chrono::system_clock::to_time_t(now);
        std::string timeStr = std::ctime(&time_t);

        json result;
        result["time"] = timeStr;
        return result.dump();
    });

    // Register a custom resource handler for dynamic content
    app.registerResourceHandler("dynamic", [](const std::string& url) -> std::string {
        if (url == "dynamic://timestamp.html") {
            const auto now = std::chrono::system_clock::now();
            const auto time_t = std::chrono::system_clock::to_time_t(now);

            return "<h1>Dynamic Content</h1>"
                   "<p>This page was generated at: " +
                   std::string(std::ctime(&time_t)) + "</p>";
        }
        throw std::runtime_error("Unknown dynamic resource");
    });

    app.run();

    return 0;
}
