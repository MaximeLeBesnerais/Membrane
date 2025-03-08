# 🧪 Membrane - The C++ and Web Tech Interface

Membrane is a lightweight framework that bridges web technologies with C++ performance. Create stunning applications with the ease of modern web development, powered by blazing-fast C++ processing.

## Preambule: License Acknowledgement

This project makes use of the following third-party libraries:

- webview (v0.12.0) - MIT License - Copyright (c) 2017 Serge Zaitsev &&Copyright (c) 2022 Steffen André Langnes: Main librairy used in this tool
- nlohmann/json (v3.11.3) - MIT License - Copyright (c) 2013-2025 Niels Lohmann: Communicate with webview and outward
- miniz (v3.0.2) - MIT License - Copyright 2013-2014 RAD Game Tools and Valve Software && Copyright 2010-2014 Rich Geldreich and Tenacious Software LLC: Used to unpack the zipped resources
- React - MIT License - Copyright (c) Meta Platforms, Inc. and affiliates: used in the example front-end
- Material-UI - MIT License - Copyright (c) 2014 Call-Em-All: Used in the example front-end

Thanks to all of them for making great open source tool that helped make mine to be.

## 🌟 Overview

Membrane provides a thin, efficient layer between your web frontend and C++ backend, enabling applications that are both beautiful and lightning-fast. Inspired by modern application frameworks but built with C++ at its core, Membrane uses native OS webviews for minimal footprint and maximum performance.

## ⚡ Features

- **Native OS Webviews**: Minimal footprint compared to bundled browser engines
- **Blazing Fast C++ Core**: Unmatched performance for computation-heavy tasks
- **Easy Frontend Development**: Use any web framework (React, Vue, Angular, etc.)
- **Bidirectional Communication**: Seamless messaging between JavaScript and C++
- **Modular Architecture**: Only include what you need to minimize application size
- **Simple API**: Clean, intuitive interfaces for both C++ and JavaScript

## 🔍 Why Membrane?

- **Lighter than Electron**: No bundled Chromium engine means smaller, faster apps
- **C++ Foundation**: Built on the familiar, powerful C++ language
- **More Flexible**: Choose your preferred web stack and security level
- **Better Developer Experience**: Use the right tools for the right job - web tech for UI, C++ for performance

## 🤔 Why Not Tauri?

Tauri is an excellent framework that shares many of Membrane's core design principles - using native webviews and separating UI from core logic.

Let's be honest - Membrane exists because I love C++ and find Rust's learning curve challenging. While Rust has amazing safety features, there's something about C++ that just clicks for me and many other developers who have spent years mastering it.

Membrane is for developers who:
- Have existing C++ codebases they want to leverage
- Feel more productive and comfortable with C++
- Prefer C++'s mature ecosystem and tooling
- Want complete control over memory management and optimization
- Need specific C++ libraries that have no Rust equivalent

Both frameworks aim to solve similar problems with different language foundations. Choose the one that best matches your skills and project requirements - there's no wrong answer!

## 🚀 Use Cases

Membrane is perfect for:

- Data-intensive applications with rich UIs
- Creative tools and media applications
- Games with web-based interfaces
- Cross-platform utilities and productivity tools
- Any project where both UI elegance and processing power matter

## 🤝 Contributing

This is my first open source project, so I'm learning as I go! I welcome contributions but please note that I might be slower than seasoned open source maintainers when it comes to reviewing and accepting PRs.

I'll personally test contributions where possible, but platform-specific improvements (especially for macOS which I don't currently have access to) may experience longer review times. Don't let this discourage you - I appreciate all help and will do my best to respond promptly.

## 🌐 Community

- **GitHub**: [github.com/MaximeLeBesnerais/Membrane](https://github.com/MaximeLeBesnerais/Membrane)
- **Discord**: not yet, will mak

## 📋 Roadmap

Membrane is under development. 
Planned features include:

- Mac Os advanced support: I only did the basics since I don't have a Mac OS. Sorry :o
- Mobile platform support: Very far, but I will graddle this build.

## 📄 License

MIT License

---

Built with ❤️ by developer, for developers.