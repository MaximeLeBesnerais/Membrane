//
// Created by maxime on 3/1/25.
//

#ifndef MEMBRANE_HPP
    #define MEMBRANE_HPP
    #include "vfs.hpp"
    #include "HttpServer.hpp"
    #include "FunctionRegistry.hpp"
    #include <webview/webview.h>
    #include "nlohmann/json.hpp"

    using json = nlohmann::json;

    /**
     * @brief The main class of the Membrane library
     * @brief Membrane is a high-level C++ wrapper around the webview library
     * @brief It provides a simple way to create a webview application with a built-in HTTP server
     * @brief and a virtual file system
     * @brief It also provides a way to register functions that can be called from the webview, allowing users to leverage C++ code from JavaScript
     */
    class Membrane {
    public:
        /**
         * @brief Construct a new Membrane object
         * @addtogroup LifeCycle
         * @param title The title of the window
         * @param entry The entry point HTML file path
         * @param width The width of the window in pixels
         * @param height The height of the window in pixels
         * @param hints Window behavior hints (WEBVIEW_HINT_NONE, WEBVIEW_HINT_MIN, etc.)
         */
        explicit Membrane(const std::string &title = "MembraneApp",
                          const std::string &entry = "index.html",
                          int width = 800,
                          int height = 600,
                          webview_hint_t hints = WEBVIEW_HINT_NONE);
        /**
         * @brief Destroy the Membrane object
         * @addtogroup LifeCycle
         */
        ~Membrane();
        /**
         * @brief Run the application
         * @addtogroup LifeCycle
         * @return true after the application has started
         * @return false on error during startup
         */
        bool run();

        /**
         * @brief Find an available port to start the HTTP server
         * @addtogroup HelperFunctions
         * @return int The port number
         */
        int findAvailablePort();

        /**
         * @brief Add 
         * 
         * @param name 
         * @param pattern 
         * @param func 
         */
        void registerFunction(const std::string &name,
            std::string pattern,
            const std::function<json(const json& args)> &func);
        /**
         * @brief Register a simple function that takes a list of arguments and returns a json object
         * @addtogroup FunctionRegistration
         * @tparam Args The argument list
         * @param name The name of the function to register
         * @param func The function to register that returns a json object
         */
        template <typename... Args>
        void registerSimpleFunction(const std::string &name,
            std::function<json(Args...)> func);
        /**
         * @brief Call a function registered with registerFunction
         * 
         * @param name 
         * @param args 
         * @return json 
         */
        json callFunction(const std::string &name, const json &args);

        /**
         * @brief Open an external URL in the default browser
         * @addtogroup HelperFunctions
         * @param url The URL to open
         */
        static void openExternal(const std::string &url);

        /**
         * @brief Save a file to disk
         * @addtogroup HelperFunctions
         * @param path The path to save the file
         * @param content The content of the file
         * @param mime The MIME type of the file
         */
        static void saveFile(const std::string &path, const std::string &content, const std::string &mime);

        /**
         * @brief Add a file to the virtual file system
         * @addtogroup VFS
         * @param path The path of the file
         * @param data The data of the file
         * @param len The length of the data
         */
        void add_vfs(const std::string &path, const unsigned char *data, unsigned int len);

        /**
         * @brief Add a custom virtual file system
         * @addtogroup VFS
         * @param name The name of the custom virtual file system
         */
        void add_custom_vfs(const std::string &name);
        
        /**
         * @brief Add a custom virtual file system with persistent storage
         * @addtogroup VFS
         * @param name The name of the custom virtual file system
         * @param path The path where the virtual file system should be persisted
         */
        void add_persistent_vfs(const std::string &name, const std::string &path);
        
        /**
         * @brief Add files to a custom virtual file system
         * @addtogroup VFS
         * @param vfs_name The name of the custom virtual file system
         * @param path The path of the file within the virtual file system
         * @param data The data of the file
         * @param len The length of the data
         */
        void add_to_custom_vfs(const std::string &vfs_name,
            const std::string &path,
            const unsigned char *data,
            unsigned int len);
        
        /**
         * @brief Save a specific virtual file system to disk
         * @addtogroup VFS
         * @param vfs_name The name of the virtual file system to save
         * @return true if the operation was successful, false otherwise
         */
        bool save_vfs_to_disk(const std::string &vfs_name);
        
        /**
         * @brief Save all virtual file systems to disk
         * @addtogroup VFS
         * @return true if the operation was successful, false otherwise
         */
        bool save_all_vfs_to_disk();

        /**
         * @brief Register a custom HTTP endpoint handler
         * @addtogroup ServerHandlers
         * @param endpoint_path The path of the endpoint to register
         * @param handler The handler function that processes the request and provides a response
         */
        void register_endpoint_handler(const std::string& endpoint_path,
            const std::function<void(const std::string&,
            const std::unordered_map<std::string, std::string>&,
            const std::string&,
            std::string&,
            std::unordered_map<std::string, std::string>&)>& handler);


    private:
        /**
         * @brief The port number to use for the connection.
         * @private
         * 
         * Defaults to 0, which typically means that the operating system
         * will choose an available port.
         */
        int _port = 0;
        /**
         * @brief The webview instance that handles the UI window
         * @private
         */
        webview::webview _window;

        /**
         * @brief The HTTP server that serves content to the webview
         * @private
         * Connects to the "/" path of the webview, essentially allowing
         * direct access to the server from the webview.
         */
        HttpServer _server;

        /**
         * @brief The main virtual file system for the application
         * @private
         * This is the default virtual file system that is used by the application.
         * It should NOT be persisted, even if you want to do your own implementation
         * of Membrane.
         */
        VirtualFileSystem _vfs;

        /**
         * @brief Collection of named custom virtual file systems
         * @private
         * This is a map of custom virtual file systems that can be added to the application.
         * Each custom VFS is mounted at its name in the server.
         */
        std::unordered_map<std::string, std::unique_ptr<VirtualFileSystem>> _custom_vfs;

        /**
         * @brief Flag indicating whether the application is running
         * @private
         * Only used in during launch procedures
         */
        bool _running = false;

        /**
         * @brief Registry for JavaScript-callable functions
         * @private
         * Allows to search for the right function to call when a JavaScript function is invoked.
         */
        FunctionRegistry _functionRegistry;

        /**
         * @brief Helper function to call a C++ function with JSON arguments
         * @private
         * @tparam Args Parameter types for the function
         * @tparam I Sequence of indices for parameter unpacking
         * @param func The function to call
         * @param args JSON arguments to pass to the function
         * @param seq Index sequence for parameter unpacking
         * @return json Result of the function call
         */
        template <typename... Args, size_t... I>
        static json callWithJsonArgsHelper(std::function<json(Args...)> func,
            const json &args, std::index_sequence<I...>);

        /**
         * @brief Call a function with arguments extracted from JSON
         * @private
         * @tparam Args Parameter types for the function
         * @param func The function to call
         * @param args JSON arguments to pass to the function
         * @return json Result of the function call
         */
        template <typename... Args>
        static json callWithJsonArgs(std::function<json(Args...)> func,
            const json &args);
    };
#endif //MEMBRANE_HPP
