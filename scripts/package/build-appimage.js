#!/usr/bin/env node

const fs = require("fs");
const path = require("path");
const { execSync, spawnSync } = require("child_process");
const packageJson = require("../../package.json");
const chalk = require("chalk");

// Configuration from package.json
const APP_NAME = packageJson.name || "membrane";
const APP_VERSION = packageJson.version || "0.1.0";
const BINARY_NAME = packageJson.binaryName || "Membrane";
const DESCRIPTION =
  packageJson.description ||
  "C++ and Web Technology Interface for desktop applications";
const AUTHOR = packageJson.author || "Maxime Le Besnerais";
// Remove email from author name
const AUTHOR_NAME =
  typeof AUTHOR === "string"
    ? AUTHOR.replace(/<.*?>/, "").trim()
    : AUTHOR.name || "Unknown Author";

// Directory structure
const SCRIPT_DIR = path.dirname(require.resolve("./build-appimage.js"));
const PROJECT_ROOT = path.resolve(SCRIPT_DIR, "../..");
const BUILD_DIR = path.resolve(PROJECT_ROOT, "build");
const DEPS_DIR = path.resolve(PROJECT_ROOT, "deps");
const PACKAGE_DIR = path.resolve(BUILD_DIR, "package");
const APPDIR = path.resolve(PACKAGE_DIR, "appdir");
const APPIMAGE_DIR = path.resolve(PACKAGE_DIR, "appimage");
const TOOLS_DIR = path.resolve(DEPS_DIR, "tools");

// Ensure required directories exist
fs.mkdirSync(DEPS_DIR, { recursive: true });
fs.mkdirSync(TOOLS_DIR, { recursive: true });
fs.mkdirSync(APPIMAGE_DIR, { recursive: true });

// Check if AppImageTool is available or download it
async function ensureAppImageTool() {
  const appimageToolPath = path.resolve(TOOLS_DIR, "appimagetool");

  // Check if we already have the tool
  if (fs.existsSync(appimageToolPath)) {
    fs.chmodSync(appimageToolPath, 0o755); // Ensure it's executable
    return appimageToolPath;
  }

  // Check if it's in the system PATH
  try {
    execSync("appimagetool --version", { stdio: "ignore" });
    return "appimagetool";
  } catch (error) {
    // Not in PATH, need to download
    console.log(chalk.yellow("appimagetool not found. Downloading it..."));

    try {
      execSync(
        `wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage" -O "${appimageToolPath}"`,
        { stdio: "inherit" }
      );
      fs.chmodSync(appimageToolPath, 0o755);
      return appimageToolPath;
    } catch (downloadError) {
      console.error(
        chalk.red(`Failed to download appimagetool: ${downloadError.message}`)
      );
      console.error(
        chalk.red(`Please install appimagetool manually and try again.`)
      );
      process.exit(1);
    }
  }
}

// Create AppDir structure and package the application
async function buildAppImage() {
  // Clean previous build
  fs.rmSync(APPDIR, { recursive: true, force: true });
  fs.mkdirSync(APPDIR, { recursive: true });

  console.log(chalk.blue("Creating AppDir structure..."));

  // Create standard AppDir structure following FHS conventions
  fs.mkdirSync(`${APPDIR}/usr/bin`, { recursive: true });
  fs.mkdirSync(`${APPDIR}/usr/lib`, { recursive: true });
  fs.mkdirSync(`${APPDIR}/usr/share/applications`, { recursive: true });
  fs.mkdirSync(`${APPDIR}/usr/share/icons/hicolor/256x256/apps`, {
    recursive: true,
  });
  fs.mkdirSync(`${APPDIR}/usr/share/metainfo`, { recursive: true });

  // Copy the binary
  const binarySource = path.resolve(BUILD_DIR, BINARY_NAME);
  const binaryDest = path.resolve(`${APPDIR}/usr/bin/${APP_NAME}`);

  if (fs.existsSync(binarySource)) {
    fs.copyFileSync(binarySource, binaryDest);
    fs.chmodSync(binaryDest, 0o755); // Make executable
  } else {
    console.error(chalk.red(`Error: Binary not found at ${binarySource}`));
    console.error(
      chalk.red(
        `Make sure to build the application before running this script.`
      )
    );
    process.exit(1);
  }

  // Icon handling
  const iconSource = path.resolve(SCRIPT_DIR, `${APP_NAME}-icon.png`);
  const iconDest = path.resolve(
    `${APPDIR}/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png`
  );

  if (!fs.existsSync(iconSource)) {
    console.warn(chalk.yellow(`Warning: No icon found at ${iconSource}.`));
    console.warn(
      chalk.yellow(`Run 'npm run create-icon' to generate a placeholder icon.`)
    );
    // Create empty files as placeholders
    fs.writeFileSync(iconDest, "");
  } else {
    fs.copyFileSync(iconSource, iconDest);
  }

  // Create desktop entry in standard location
  const desktopEntry = `[Desktop Entry]
Name=${BINARY_NAME}
GenericName=Web Interface Framework
Comment=${DESCRIPTION}
Exec=${APP_NAME}
Icon=${APP_NAME}
Terminal=false
Type=Application
Categories=Development;
Keywords=webview;gui;framework;
`;

  const desktopFilePath = `${APPDIR}/usr/share/applications/${APP_NAME}.desktop`;
  fs.writeFileSync(desktopFilePath, desktopEntry);

  // Create AppStream metadata
  const currentDate = new Date().toISOString().split("T")[0]; // YYYY-MM-DD
  const appStreamId = `io.github.${APP_NAME.toLowerCase()}`;

  const appStreamMetadata = `<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>${appStreamId}</id>
  <name>${BINARY_NAME}</name>
  <summary>${DESCRIPTION}</summary>
  <metadata_license>MIT</metadata_license>
  <project_license>MIT</project_license>
  <description>
    <p>
      Membrane is a lightweight framework that bridges web technologies with C++ performance.
      Create stunning applications with the ease of modern web development, powered by
      blazing-fast C++ processing.
    </p>
    <p>
      Features:
    </p>
    <ul>
      <li>Native OS Webviews: Minimal footprint compared to bundled browser engines</li>
      <li>Blazing Fast C++ Core: Unmatched performance for computation-heavy tasks</li>
      <li>Easy Frontend Development: Use any web framework (React, Vue, Angular, etc.)</li>
      <li>Bidirectional Communication: Seamless messaging between JavaScript and C++</li>
    </ul>
  </description>
  <launchable type="desktop-id">${APP_NAME}.desktop</launchable>
  <url type="homepage">https://github.com/MaximeLeBesnerais/Membrane</url>
  <url type="bugtracker">https://github.com/MaximeLeBesnerais/Membrane/issues</url>
  <provides>
    <binary>${APP_NAME}</binary>
  </provides>
  <releases>
    <release version="${APP_VERSION}" date="${currentDate}">
      <description>
        <p>Initial release</p>
      </description>
    </release>
  </releases>
  <content_rating type="oars-1.1" />
  <developer_name>${AUTHOR_NAME}</developer_name>
</component>
`;

  fs.writeFileSync(
    `${APPDIR}/usr/share/metainfo/${APP_NAME}.appdata.xml`,
    appStreamMetadata
  );

  // Create standard symlinks for AppDir structure

  // 1. Create AppRun script
  const appRunScript = `#!/bin/bash
# Get the directory where this AppRun script is located
APPDIR="\$(dirname "\$(readlink -f "\${0}")")"

# Set LD_LIBRARY_PATH to include the libraries in the AppDir
export LD_LIBRARY_PATH="\$APPDIR/usr/lib:\$LD_LIBRARY_PATH"

# Execute the main binary
exec "\$APPDIR/usr/bin/${APP_NAME}" "\$@"
`;

  fs.writeFileSync(`${APPDIR}/AppRun`, appRunScript);
  fs.chmodSync(`${APPDIR}/AppRun`, 0o755);

  // 2. Create symlinks for .DirIcon and desktop file in root directory
  try {
    // Create root desktop file symlink
    fs.symlinkSync(
      `usr/share/applications/${APP_NAME}.desktop`,
      `${APPDIR}/${APP_NAME}.desktop`
    );

    // Create root icon symlink
    fs.symlinkSync(
      `usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png`,
      `${APPDIR}/${APP_NAME}.png`
    );

    // Create .DirIcon symlink
    fs.symlinkSync(`${APP_NAME}.png`, `${APPDIR}/.DirIcon`);
  } catch (error) {
    console.warn(
      chalk.yellow(`Warning: Could not create symlinks: ${error.message}`)
    );
    // Fall back to copying files if symlinks fail
    fs.copyFileSync(
      `${APPDIR}/usr/share/applications/${APP_NAME}.desktop`,
      `${APPDIR}/${APP_NAME}.desktop`
    );
    fs.copyFileSync(
      `${APPDIR}/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png`,
      `${APPDIR}/${APP_NAME}.png`
    );
    fs.copyFileSync(`${APPDIR}/${APP_NAME}.png`, `${APPDIR}/.DirIcon`);
  }

  // Set permissions
  fs.chmodSync(`${APPDIR}/usr/bin/${APP_NAME}`, 0o755);
  execSync(`find "${APPDIR}/usr/share" -type f -exec chmod 644 {} \\;`);

  console.log(chalk.blue("Checking required libraries..."));

  // Check if we need to bundle any libraries
  try {
    const lddOutput = execSync(
      `ldd "${APPDIR}/usr/bin/${APP_NAME}"`
    ).toString();
    const libraries = lddOutput
      .split("\n")
      .filter((line) => line.includes("=>") && line.includes("/"))
      .map((line) => line.split("=>")[1].trim().split(" ")[0]);

    // Copy system libraries that might be missing on some systems
    for (const lib of libraries) {
      // Skip some common system libraries that should be available
      if (
        lib.includes("libc.so") ||
        lib.includes("libstdc++.so") ||
        lib.includes("libdl.so") ||
        lib.includes("libpthread.so") ||
        lib.includes("libm.so") ||
        lib.includes("libgcc_s.so") ||
        lib.includes("librt.so") ||
        lib.includes("/usr/lib")
      ) {
        continue;
      }

      // Copy the library
      console.log(`Bundling: ${lib}`);
      fs.copyFileSync(lib, path.join(`${APPDIR}/usr/lib`, path.basename(lib)));
    }
  } catch (error) {
    console.warn(
      chalk.yellow(
        `Warning: Could not analyze binary dependencies: ${error.message}`
      )
    );
  }

  console.log(
    chalk.yellow(
      "Note: For a production build, consider using linuxdeploy for comprehensive dependency bundling."
    )
  );

  // Build the AppImage
  console.log(chalk.blue("Building AppImage..."));
  const APPIMAGE_FILE = `${APP_NAME}-${APP_VERSION}-x86_64.AppImage`;
  const APPIMAGE_PATH = path.resolve(APPIMAGE_DIR, APPIMAGE_FILE);

  // Get path to appimagetool
  const appimageToolPath = await ensureAppImageTool();

  try {
    // Use appimagetool to build the AppImage (with --no-appstream to skip validation)
    const result = spawnSync(
      appimageToolPath,
      ["--no-appstream", APPDIR, APPIMAGE_PATH],
      {
        stdio: "inherit",
        shell: true,
      }
    );

    if (result.status === 0) {
      console.log(
        chalk.green(`✅ AppImage created successfully: ${APPIMAGE_PATH}`)
      );
      // Make it executable
      fs.chmodSync(APPIMAGE_PATH, 0o755);
      // Calculate package size
      const appimageSize = execSync(`du -h "${APPIMAGE_PATH}" | cut -f1`)
        .toString()
        .trim();
      console.log(`   Size: ${appimageSize}`);
    } else {
      console.error(chalk.red(`❌ Failed to create AppImage`));
      process.exit(1);
    }
  } catch (error) {
    console.error(chalk.red(`Error building AppImage: ${error.message}`));
    process.exit(1);
  }
}

buildAppImage().catch((error) => {
  console.error(chalk.red(`Error: ${error.message}`));
  process.exit(1);
});
