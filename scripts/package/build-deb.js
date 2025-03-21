#!/usr/bin/env node

const fs = require("fs");
const path = require("path");
const { execSync } = require("child_process");
const packageJson = require("../../package.json");
const chalk = require("chalk");

// Configuration from package.json
const APP_NAME = packageJson.name || "membrane";
const PACKAGE_NAME = APP_NAME.toLowerCase();
const PACKAGE_VERSION = packageJson.version || "0.1.0";
const BINARY_NAME = packageJson.binaryName || "Membrane";
const DESCRIPTION =
  packageJson.description ||
  "C++ and Web Technology Interface for desktop applications";
const HOMEPAGE =
  packageJson.homepage || "https://github.com/MaximeLeBesnerais/Membrane";
const AUTHOR = packageJson.author || "Maxime Le Besnerais <maxime@example.com>";

// Fixed configuration
const ARCHITECTURE = "amd64"; // x86_64
const SECTION = "devel";
const PRIORITY = "optional";
// More flexible dependencies that work across various distributions
const DEPENDS =
  "libc6, libgtk-3-0, libwebkit2gtk-4.0-37 | libwebkit2gtk-4.1-0 | libwebkit2gtk-4.0";

// Directory structure
const SCRIPT_DIR = path.dirname(require.resolve("./build-deb.js"));
const PROJECT_ROOT = path.resolve(SCRIPT_DIR, "../..");
const BUILD_DIR = path.resolve(PROJECT_ROOT, "build");
const PACKAGE_DIR = path.resolve(BUILD_DIR, "package");
const PACKAGE_BUILD_DIR = path.resolve(PACKAGE_DIR, "deb-build");
const DEB_DIR = path.resolve(PACKAGE_DIR, "deb");

// Create directory structure
console.log(chalk.blue("Creating Debian package structure..."));
fs.rmSync(PACKAGE_BUILD_DIR, { recursive: true, force: true });
fs.mkdirSync(PACKAGE_BUILD_DIR, { recursive: true });
fs.mkdirSync(DEB_DIR, { recursive: true });

// Create DEBIAN directory
fs.mkdirSync(`${PACKAGE_BUILD_DIR}/DEBIAN`, { recursive: true });

// Create the control file
const controlFile = `Package: ${PACKAGE_NAME}
Version: ${PACKAGE_VERSION}
Section: ${SECTION}
Priority: ${PRIORITY}
Architecture: ${ARCHITECTURE}
Depends: ${DEPENDS}
Maintainer: ${AUTHOR}
Description: ${DESCRIPTION}
 Membrane is a lightweight framework that bridges web technologies with C++ performance.
 Create stunning applications with the ease of modern web development,
 powered by blazing-fast C++ processing.
 .
 Uses native OS webviews for minimal footprint and maximum performance.
Homepage: ${HOMEPAGE}
`;

fs.writeFileSync(`${PACKAGE_BUILD_DIR}/DEBIAN/control`, controlFile);

// Create the installation directories
fs.mkdirSync(`${PACKAGE_BUILD_DIR}/usr/bin`, { recursive: true });
fs.mkdirSync(`${PACKAGE_BUILD_DIR}/usr/share/${PACKAGE_NAME}`, {
  recursive: true,
});
fs.mkdirSync(`${PACKAGE_BUILD_DIR}/usr/share/applications`, {
  recursive: true,
});
fs.mkdirSync(`${PACKAGE_BUILD_DIR}/usr/share/icons/hicolor/256x256/apps`, {
  recursive: true,
});
fs.mkdirSync(`${PACKAGE_BUILD_DIR}/usr/share/doc/${PACKAGE_NAME}`, {
  recursive: true,
});

// Copy the binary
const binarySource = path.resolve(BUILD_DIR, BINARY_NAME);
const binaryDest = path.resolve(`${PACKAGE_BUILD_DIR}/usr/bin/${PACKAGE_NAME}`);

if (fs.existsSync(binarySource)) {
  fs.copyFileSync(binarySource, binaryDest);
  fs.chmodSync(binaryDest, 0o755); // Make executable
} else {
  console.error(chalk.red(`Error: Binary not found at ${binarySource}`));
  console.error(
    chalk.red(`Make sure to build the application before running this script.`)
  );
  process.exit(1);
}

// Copy the icon (will use the placeholder if available or create a blank one)
const iconSource = path.resolve(SCRIPT_DIR, "icon.png");
const iconDest = path.resolve(
  `${PACKAGE_BUILD_DIR}/usr/share/icons/hicolor/256x256/apps/${PACKAGE_NAME}.png`
);

if (!fs.existsSync(iconSource)) {
  console.warn(chalk.yellow(`Warning: No icon found at ${iconSource}.`));
  console.warn(
    chalk.yellow(`Run 'npm run create-icon' to generate a placeholder icon.`)
  );
  // Create a small empty file as a placeholder
  fs.writeFileSync(iconDest, "");
} else {
  fs.copyFileSync(iconSource, iconDest);
}

// Create a desktop entry
const desktopEntry = `[Desktop Entry]
Name=Membrane
GenericName=Web Interface Framework
Comment=${DESCRIPTION}
Exec=${PACKAGE_NAME}
Icon=${PACKAGE_NAME}
Terminal=false
Type=Application
Categories=Development;
Keywords=webview;gui;framework;
`;

fs.writeFileSync(
  `${PACKAGE_BUILD_DIR}/usr/share/applications/${PACKAGE_NAME}.desktop`,
  desktopEntry
);

// Copy documentation
const licenseSource = path.resolve(PROJECT_ROOT, "LICENSE");
const readmeSource = path.resolve(PROJECT_ROOT, "readme.md");

if (fs.existsSync(licenseSource)) {
  fs.copyFileSync(
    licenseSource,
    `${PACKAGE_BUILD_DIR}/usr/share/doc/${PACKAGE_NAME}/copyright`
  );
}

if (fs.existsSync(readmeSource)) {
  fs.copyFileSync(
    readmeSource,
    `${PACKAGE_BUILD_DIR}/usr/share/doc/${PACKAGE_NAME}/README.md`
  );
}

// Set permissions
fs.chmodSync(`${PACKAGE_BUILD_DIR}/usr/bin/${PACKAGE_NAME}`, 0o755);
execSync(`find "${PACKAGE_BUILD_DIR}/DEBIAN" -type f -exec chmod 644 {} \\;`);
execSync(
  `find "${PACKAGE_BUILD_DIR}/usr/share" -type f -exec chmod 644 {} \\;`
);

// Build the package
console.log(chalk.blue("Building Debian package..."));
const debFile = `${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCHITECTURE}.deb`;
const debPath = path.resolve(DEB_DIR, debFile);

try {
  execSync(`dpkg-deb --build "${PACKAGE_BUILD_DIR}" "${debPath}"`, {
    stdio: "inherit",
  });

  // Calculate package size and md5sum
  const debSize = execSync(`du -h "${debPath}" | cut -f1`).toString().trim();
  const debMd5 = execSync(`md5sum "${debPath}" | cut -d ' ' -f1`)
    .toString()
    .trim();

  console.log(chalk.green(`✅ Package created successfully: ${debPath}`));
  console.log(`   Size: ${debSize}`);
  console.log(`   MD5:  ${debMd5}`);
} catch (error) {
  console.error(chalk.red(`❌ Failed to create package: ${error.message}`));
  process.exit(1);
}
