//
// Created by maxime on 2/27/25.
//

#ifndef MEMBRANE_HPP
    #define MEMBRANE_HPP
    #include "webview/webview.h"
    #include <nlohmann/json.hpp>
    #include <unordered_map>
    #include <string>
    #include <functional>
    #include <memory>
    #include <iostream>
    using json = nlohmann::json;

    class Membrane {
    public:
        using ResourceCallback = std::function<std::string(const std::string&)>;
        using APICallback = std::function<std::string(const std::string&)>;

        // Constructor
        explicit Membrane(bool debug = false,
            const std::string &title = "Membrane App",
            int width = 1024,
            int height = 768);

        // Destructor
        ~Membrane() = default;

        // Prevent copying
        Membrane(const Membrane&) = delete;
        Membrane& operator=(const Membrane&) = delete;

        // Add a virtual resource to the in-memory filesystem
        void addResource(const std::string& url, const std::string& content);

        // Add resources from a directory with a given prefix
        // void addResourceDirectory(const std::string& directoryPath, const std::string& urlPrefix);

        // Register a resource handler for a specific URL scheme
        void registerResourceHandler(const std::string& scheme, const ResourceCallback &callback);

        // Register a JavaScript API method
        void registerAPI(const std::string& name, APICallback callback) const;

        // Set the application title
        void setTitle(const std::string& title) const;

        // Set window size
        void setSize(int width, int height, int hints = 0) const;

        // Initialize and run with bootstrap HTML
        void run(const std::string& bootstrapUrl = "");

    private:
        std::unique_ptr<webview::webview> m_webview;
        std::unordered_map<std::string, std::string> m_resources;
        std::unordered_map<std::string, ResourceCallback> m_resourceHandlers;

        // Helper to parse URL from JSON request
        static std::string parseUrl(const std::string& req);

        // Extract scheme from URL
        static std::string getScheme(const std::string& url);

        // Generate the bootstrap HTML that initializes the framework
        static std::string getBootstrapHtml(const std::string& initialUrl);

        // Default bootstrap HTML if no initial URL is provided
        static std::string getDefaultBootstrapHtml();
    };

#endif //MEMBRANE_HPP
