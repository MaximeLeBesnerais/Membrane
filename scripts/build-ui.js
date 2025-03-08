const fs = require('fs');
const { execSync } = require('child_process');
const chalk = require('chalk');
const os = require('os');

const CACHE_FILE = "src-react/build-hash.txt";
const isWindows = os.platform() === 'win32';
const isMac = os.platform() === 'darwin';

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
    console.log(chalk.blue("Building React app..."));
    if (isWindows)
        execSync("cd src-react && bun i && bun run package:win", { stdio: 'inherit' });
    else if (isMac)
        execSync("cd src-react && bun i && bun run package:mac", { stdio: 'inherit' });
    else
        execSync("cd src-react && bun i && bun run package:linux", { stdio: 'inherit' });
    fs.writeFileSync(CACHE_FILE, newHash);
}
