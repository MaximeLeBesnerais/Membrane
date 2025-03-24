# 🧪 Membrane: 轻量级C++和Web技术接口

Membrane在现代Web前端和原生C++性能之间提供了一个轻薄、高效的层。创建具有Web技术UI能力的响应式应用程序，同时利用C++的速度进行核心操作。

## 🚀 概述

Membrane是一个轻量级C++框架，使开发者能够使用Web技术构建桌面应用程序，同时避免捆绑浏览器引擎带来的开销。为以下开发者设计：

- 有C++经验并希望将其用于后端操作
- 需要Web UI功能但想要比Electron更高效的解决方案
- 相比Rust更偏好使用C++开发原生桌面应用程序

## 🔄 各平台支持状态

- **Linux**: ✅ 完全支持，尚未进行单元测试
- **Windows**: 🚧 计划支持（主要是构建系统修复）
- **macOS**: ⚠️ 初步支持 - 完全未测试（无可用的macOS机器）

## ⚡ 核心特性

- **原生OS网页视图**: 使用系统原生webview以最小化资源使用
- **C++后端**: 利用C++的全部威力处理性能关键任务
- **双向通信**: JavaScript和C++之间的无缝集成
- **虚拟文件系统**: 高效传递Web资产和资源
- **模块化架构**: 在应用程序中仅包含所需功能

## 📦 核心依赖

- **CMake**: 3.30或更高版本用于构建系统
- **C++编译器**: 支持C++20（GCC 10+, Clang 10+）
- **WebKit2GTK**: 用于Linux系统（`libwebkit2gtk-4.0-dev`）
- **标准库**: pthread, 标准C++库
- **Node.js/Bun**: 用于前端开发

## 🔍 为什么选择Membrane?

### 与Electron相比

- **显著降低内存使用**: 无捆绑的Chromium引擎
- **更小的应用程序尺寸**: 通常为1-5MB，而Electron应用为100-200MB*
- **原生集成**: 使用操作系统的原生渲染能力
- **C++基础**: 直接访问C++库和性能
- **资源效率**: 在低端硬件上提供更好的电池寿命和性能

### 与Tauri相比

Tauri是一个优秀的框架，直接启发了Membrane。事实上：

- **Tauri更好**: 更成熟，支持更好，由更大的社区积极开发
- **Membrane存在是因为C++**: 我个人更喜欢C++而非Rust - 这是唯一原因
- **相似目标**: 两者都旨在使用Web技术创建轻量级桌面应用
- **选择Tauri**: 如果你对Rust感到舒适或从头开始，推荐使用Tauri

## 🛠️ 快速开始

1. 克隆仓库:
   ```bash
   git clone https://gitcode.com/xyj_maxime/Membrane
   cd Membrane
   ```

2. 用你的Web前端代码替换`src-react`文件夹或使用提供的演示

3. 确保你的`src-react`文件夹包含:
   - 用于Linux/macOS资源编译的`Gen.sh`
   - 用于Windows的`Gen.ps1`
   - 适用于构建和打包的正确package.json脚本
   - 构建输出应该保存到`src-react/dist`目录

4. 构建应用程序:
   ```bash
   # 安装依赖
   npm run deps
   
   # 开发模式
   npm run dev
   
   # 生产构建
   npm run build
   ```

## 📝 创建你的第一个应用

Membrane应用由两部分组成:

1. **前端** (在`src-react/`): 任何你喜欢的Web技术(React, Vue, Angular等)
2. **后端** (在`lib/`和`src/`): 核心功能的C++代码

前端通过注册的JavaScript函数与后端通信，这些函数调用C++方法。例如:

```javascript
// 在你的React/JS代码中
window.membrane_readFile("/path/to/file")
  .then(result => console.log(result));
```

```cpp
// 在你的C++代码中
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

## 小型路线图
- **测试策略**: 为核心功能添加单元测试，然后为所有功能添加测试，以及行为测试
- **Windows支持**: 添加Windows支持，包括构建系统修复
- **反向设置**: 我希望用户能将Membrane文件夹添加到他们的项目中并作为库使用，而非必须克隆仓库

## 反向设置逻辑(计划中)

未来，Membrane将支持"反向设置"工作流，允许你以最小的项目结构更改将Membrane添加到任何现有的Web应用程序:

1. 只需将Membrane目录添加到你的Web项目
2. 在package.json中为Membrane命令添加几行:

```json
"scripts": {
  "membrane:dev": "cross-env DEV_MODE=ON node Membrane/scripts/build-cpp.js watch",
  "membrane:build": "npm run build && node Membrane/scripts/build-cpp.js",
  "membrane:package": "npm run build && node Membrane/scripts/package.js"
}
```

3. 继续使用你的常规Web架构和构建流程

这种方法意味着你不需要采用特殊的目录结构或大幅修改现有的Web应用程序。Membrane将适应你的项目，而不是强制你的项目适应Membrane。

目标是使集成尽可能无缝，允许你以最小的干扰对现有工作流程添加原生桌面功能到Web应用程序。


## 📜 许可和归属

本项目基于MIT许可发布。

### 第三方库

Membrane使用以下库:

- **webview** (v0.12.0) - MIT许可 - [GitHub](https://github.com/webview/webview) - [GitCode镜像](https://gitcode.com/gh_mirrors/we/webview)
- **nlohmann/json** (v3.11.3) - MIT许可 - [GitHub](https://github.com/nlohmann/json) - [GitCode镜像](https://gitcode.com/gh_mirrors/js/json)
- **miniz** (v3.0.2) - MIT许可 - [GitHub](https://github.com/richgel999/miniz) - [GitCode镜像](https://gitcode.com/gh_mirrors/mi/miniz)

### CMake配置说明

用户**可能**需要修改CMake配置，但我目前无法亲自测试。如果中国用户在GitCode上提出请求，我将基于需求添加相应的CMake标志。

如果您在使用过程中遇到任何问题，请在GitCode项目页面上提出问题，我将尽快解决。


## 🤝 贡献

欢迎贡献！作为一个新的开源项目，我在边做边学，所以请对PR审核保持耐心。

特别需要帮助的领域:
- Windows支持改进
- macOS测试和增强
- 文档和示例
- 性能优化

## 🌟 致谢

由Maxime Le Besnerais用❤️构建，为那些寻找Electron高效、注重性能替代方案的开发者。

---
<details>
<summary>应用程序大小说明</summary>

*注意：应用程序的大小可能因使用的打包方法而异。例如，<span style="color:#e74c3c">AppImage包往往更大</span>，而<span style="color:#27ae60">deb包</span>因其自包含的特性更小。我的一些测试生成了约<span style="color:#e74c3c">~130MB</span>的AppImage，而deb包约为<span style="color:#27ae60">2MB</span>。这不是Membrane的限制，而是所使用打包方法的特性。应用程序的实际大小将取决于包中包含的资源和库。可能还有一些可能的改进来减小AppImage的大小，但我还没有探索过。<span style="color:#3498db">该项目的目标</span>是提供一个轻量级框架来构建应用程序，最终包的大小将取决于开发者选择的特定用例和打包方法。重点是提供一个框架，让开发者能够创建<span style="color:#f39c12">高效的应用程序</span>，而不受完整浏览器引擎的开销影响。*
</details>

*注意：此项目是为了支持Oyster Security（另一个开源项目）并解决对轻量级桌面应用程序框架的需求，该框架利用C++性能与现代Web UI。*