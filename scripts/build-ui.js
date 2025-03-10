const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');
const chalk = require('chalk');

const DIST_DIR = "src-react/dist";
const CACHE_FILE = "src-react/build-hash.txt";

// Platform configuration map
const platformConfig = {
  win32: {
    name: 'windows',
    buildCommand: 'cd src-react && bun i && bun run package:windows'
  },
  darwin: {
    name: 'darwin', 
    buildCommand: 'cd src-react && bun i && bun run package:darwin'
  },
  linux: {
    name: 'linux',
    buildCommand: 'cd src-react && bun i && bun run package:linux'
  }
};

// Get current platform configuration
const currentPlatform = platformConfig[process.platform] || platformConfig.linux;
console.log(chalk.blue(`Building UI for ${currentPlatform.name}...`));

// Compute hash of all source files
function computeHash(dir) {
    const files = fs.readdirSync(dir, { withFileTypes: true });
    return files.map(file => {
        const filePath = `${dir}/${file.name}`;
        if (file.isDirectory()) return computeHash(filePath);
        return fs.existsSync(filePath) ? fs.statSync(filePath).mtimeMs : '';
    }).join('');
}

// Check if rebuild is needed
const newHash = computeHash("src-react/src");
let prevHash = "";
if (fs.existsSync(CACHE_FILE)) prevHash = fs.readFileSync(CACHE_FILE, 'utf8');

if (newHash === prevHash) {
    console.log(chalk.green("Skipping React build (no changes detected)."));
} else {
    console.log(chalk.blue(`Building React app for ${currentPlatform.name}...`));
    execSync(currentPlatform.buildCommand, { stdio: 'inherit' });
    fs.writeFileSync(CACHE_FILE, newHash);
}