# 🧪 Membrane: Lightweight C++ and Web Tech Interface

Membrane provides a thin, efficient layer between modern web frontends and native C++ performance. Create responsive applications with the UI capabilities of web technologies while leveraging C++'s speed for core operations.

## 🚀 Overview

Membrane is a lightweight C++ framework that enables developers to build desktop applications with web technologies while avoiding the overhead of bundled browser engines. Built for developers who:

- Have experience with C++ and want to use it for backend operations
- Need web UI capabilities but want a more efficient solution than Electron
- Prefer C++ over Rust for native desktop applications

## 🔄 Status by Platform

- **Linux**: ✅ Fully supported, not unit tested yet
- **Windows**: 🚧 Support planned (primarily build system fixes)
- **macOS**: ⚠️ Premise support - entirely untested (no macOS machine available)

## ⚡ Core Features

- **Native OS Webviews**: Uses your system's native webview for minimal resource usage
- **C++ Backend**: Harness the full power of C++ for performance-critical tasks
- **Bidirectional Communication**: Seamless integration between JavaScript and C++
- **Virtual Filesystem**: Web assets and resources delivered efficiently
- **Modular Architecture**: Include only what you need in your application

## 📦 Core Dependencies

- **CMake**: 3.30 or higher for build system
- **C++ Compiler**: C++20 capable (GCC 10+, Clang 10+)
- **WebKit2GTK**: For Linux systems (`libwebkit2gtk-4.0-dev`)
- **Standard Libraries**: pthread, standard C++ libraries
- **Node.js/Bun**: For frontend development

## 🔍 Why Membrane?

### Compared to Electron

- **Significantly Lower Memory Usage**: No bundled Chromium engine
- **Smaller Application Size**: Typically 1-5MB vs. 100-200MB for Electron apps*
- **Native Integration**: Uses the OS's native rendering capabilities
- **C++ Foundation**: Direct access to C++ libraries and performance
- **Resource Efficiency**: Better battery life and performance on lower-end hardware

### Compared to Tauri

Tauri is an excellent framework that directly inspired Membrane. In truth:

- **Tauri is Better**: More mature, better supported, and actively developed by a larger community
- **Membrane Exists Because of C++**: I personally prefer C++ over Rust - that's the only reason
- **Similar Goals**: Both aim to create lightweight desktop apps with web technologies
- **Choose Tauri**: If you're comfortable with Rust or starting fresh, Tauri is recommended

## 🛠️ Quick Start

1. Clone the repository:
   ```bash
   git clone https://github.com/MaximeLeBesnerais/Membrane
   cd Membrane
   ```

2. Replace the `src-react` folder with your web frontend code or use the provided demo

3. Make sure your `src-react` folder contains:
   - `Gen.sh` for resource compilation on Linux/macOS
   - `Gen.ps1` for Windows
   - Proper package.json scripts for building and packaging
   - Build output should go to `src-react/dist` directory

4. Build the application:
   ```bash
   # Install dependencies
   npm run deps
   
   # Development mode
   npm run dev
   
   # Production build
   npm run build
   ```

## 📝 Creating Your First App

Membrane applications consist of two main parts:

1. **Frontend** (in `src-react/`): Any web technology you prefer (React, Vue, Angular, etc.)
2. **Backend** (in `lib/` and `src/`): C++ code for core functionality

The frontend communicates with the backend using registered JavaScript functions that call C++ methods. For example:

```javascript
// In your React/JS code
window.membrane_readFile("/path/to/file")
  .then(result => console.log(result));
```

```cpp
// In your C++ code
app.registerFunction("membrane_readFile", [](const json& args) -> json {
    std::string path = args[0].as_string();
    try {
        std::string content = readFile(path);
        return json({
            {"status", "success"},
            {"message", "File read successfully"},
            {"data", content}
        });
    } catch (const std::exception& e) {
        return retObj("error", e.what());
    }
});
```

## Mini Roadmap
- **Testing Policy**: Add unit tests for core features, then all features, as well as behavioral tests
- **Windows Support**: Add support for Windows, including build system fixes
- **Reverse setup**: I want users to add a Membrane folder to their project and use it as a library, rather than having to clone the repository

## Reverse Setup Logic (Planned)

In the future, Membrane will support a "reverse setup" workflow that allows you to add Membrane to any existing web application with minimal changes to your project structure:

1. Simply add the Membrane directory to your web project
2. Add a few lines to your package.json for Membrane commands:

```json
"scripts": {
  "membrane:dev": "cross-env DEV_MODE=ON node Membrane/scripts/build-cpp.js watch",
  "membrane:build": "npm run build && node Membrane/scripts/build-cpp.js",
  "membrane:package": "npm run build && node Membrane/scripts/package.js"
}
```

3. Continue using your normal web architecture and build process

This approach means you won't need to adopt a special directory structure or significantly modify your existing web application. Membrane will adapt to your project instead of forcing your project to adapt to Membrane.

The goal is to make integration as seamless as possible, allowing you to add native desktop capabilities to web applications with minimal disruption to your existing workflow.


## 📜 License and Attribution

This project is released under the MIT License.

### Third-Party Libraries

Membrane makes use of the following libraries:

- **webview** (v0.12.0) - MIT License - [GitHub](https://github.com/webview/webview)
- **nlohmann/json** (v3.11.3) - MIT License - [GitHub](https://github.com/nlohmann/json)
- **miniz** (v3.0.2) - MIT License - [GitHub](https://github.com/richgel999/miniz)

## 🤝 Contributing

Contributions are welcome! As a new open source project, I'm learning as I go, so please be patient with PR reviews.

Areas particularly needing help:
- Windows support improvements
- macOS testing and enhancements
- Documentation and examples
- Performance optimizations

## 🌟 Acknowledgments

Built with ❤️ by Maxime Le Besnerais for developers looking for an efficient, performance-focused alternative to Electron.

---
<details>
<summary>Application Size Notes</summary>

*Note: The size of the application may vary based on the packaging method used. For example, <span style="color:#e74c3c">AppImage packages tend to be larger</span> than <span style="color:#27ae60">deb packages</span> due to their self-contained nature. Some of my test made AppImage of <span style="color:#e74c3c">~130MB</span>, while deb packages were around <span style="color:#27ae60">2MB</span>. This is not a limitation of Membrane but rather a characteristic of the packaging method used. The actual size of the application will depend on the resources and libraries included in the package. There might also be some possible improvements to reduce the size of the AppImage, but I haven't explored that yet. The <span style="color:#3498db">goal of this project</span> is to provide a lightweight framework for building applications, and the size of the final package will depend on the specific use case and packaging method chosen by the developer. The focus here is on providing a framework that allows developers to create <span style="color:#f39c12">efficient applications</span> without the overhead of a full browser engine.*
</details>

*Note: This project was created to support Oyster Security (another open source project) and address the need for a lightweight desktop application framework that leverages C++ performance with modern web UIs.*