#!/usr/bin/env node

const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const { execSync } = require('child_process');
const chalk = require('chalk');

// External libraries config
const DEPENDENCIES = {
  "nlohmann/json": {
    url: "https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz",
    hashFile: "deps/json.hash"
  },
  "webview": {
    repo: "https://github.com/webview/webview.git",
    tag: "0.12.0",
    hashFile: "deps/webview.hash"
  }
};

// Create deps directory if it doesn't exist
if (!fs.existsSync('deps')) {
  fs.mkdirSync('deps', { recursive: true });
}

function computeFileHash(filePath) {
  const content = fs.readFileSync(filePath);
  return crypto.createHash('sha256').update(content).digest('hex');
}

function saveHash(hashFile, hash) {
  fs.writeFileSync(hashFile, hash);
}

function getStoredHash(hashFile) {
  if (fs.existsSync(hashFile)) {
    return fs.readFileSync(hashFile, 'utf8').trim();
  }
  return '';
}

function downloadDependencies() {
  console.log(chalk.blue('Checking external dependencies...'));
  
  // Track if any dependency was updated
  let updated = false;

  // CMakeLists.txt hash check
  const cmakeListsPath = path.join(__dirname, '../CMakeLists.txt');
  const cmakeListsHash = computeFileHash(cmakeListsPath);
  const cmakeListsHashFile = path.join(__dirname, '../deps/cmakelists.hash');
  const storedCmakeHash = getStoredHash(cmakeListsHashFile);
  
  if (cmakeListsHash !== storedCmakeHash) {
    console.log(chalk.yellow('CMakeLists.txt has changed, forcing dependency check...'));
    updated = true;
    saveHash(cmakeListsHashFile, cmakeListsHash);
  }

  // Check each dependency
  Object.entries(DEPENDENCIES).forEach(([name, config]) => {
    const hashFilePath = path.join(__dirname, '..', config.hashFile);
    const currentHash = getStoredHash(hashFilePath);
    
    // If url-based dependency
    if (config.url) {
      const urlHash = crypto.createHash('sha256').update(config.url).digest('hex');
      
      if (currentHash !== urlHash || updated) {
        console.log(chalk.green(`Updating ${name}...`));
        try {
          // Download using FetchContent in CMake, we'll just mark as updated
          saveHash(hashFilePath, urlHash);
          updated = true;
        } catch (error) {
          console.error(chalk.red(`Failed to update ${name}: ${error.message}`));
        }
      } else {
        console.log(chalk.green(`${name} is up to date.`));
      }
    }
    
    // If git-based dependency
    if (config.repo) {
      const gitHash = crypto.createHash('sha256').update(`${config.repo}#${config.tag}`).digest('hex');
      
      if (currentHash !== gitHash || updated) {
        console.log(chalk.green(`Updating ${name}...`));
        try {
          // Download using FetchContent in CMake, we'll just mark as updated
          saveHash(hashFilePath, gitHash);
          updated = true;
        } catch (error) {
          console.error(chalk.red(`Failed to update ${name}: ${error.message}`));
        }
      } else {
        console.log(chalk.green(`${name} is up to date.`));
      }
    }
  });
  
  return updated;
}

try {
  const updated = downloadDependencies();
  
  if (updated) {
    console.log(chalk.blue('Dependencies updated. Running CMake configure...'));
    // This will force CMake to re-run FetchContent on next build
    console.log('Dependencies checked and ready for CMake to process.');
  } else {
    console.log(chalk.blue('All dependencies are up to date.'));
  }
} catch (error) {
  console.error(chalk.red(`Error: ${error.message}`));
  process.exit(1);
}
