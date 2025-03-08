#!/usr/bin/env node

const fs = require('fs');
const path = require('path');
const { execSync, spawn } = require('child_process');
const packageJson = require('../package.json');
const BINARY_NAME = packageJson.binaryName || process.env.BINARY_NAME || 'Membrane';
const chalk = require('chalk');

// Platform configuration map
const platformConfig = {
  win32: {
    name: 'windows',
    genCommand: 'powershell -File',
    execExtension: '.exe',
    pathSeparator: '\\',
  },
  darwin: {
    name: 'darwin',
    genCommand: 'bash',
    execExtension: '',
    pathSeparator: '/',
  },
  linux: {
    name: 'linux',
    genCommand: 'bash',
    execExtension: '',
    pathSeparator: '/',
  }
};

// Get current platform configuration
const currentPlatform = platformConfig[process.platform] || platformConfig.linux;
console.log(chalk.blue(`Building ${BINARY_NAME} for ${currentPlatform.name}...`));

// Configuration
const IS_DEV = process.env.DEV_MODE === 'ON';
const BUILD_DIR = IS_DEV ? 'build-dev' : 'build';
const WATCH_MODE = process.argv.includes('watch');

// Platform-specific paths
const PLATFORM_CMAKE = `scripts/platforms/${currentPlatform.name}/${currentPlatform.name}.cmake`;
const PLATFORM_GEN_SCRIPT = `scripts/platforms/${currentPlatform.name}/gen${currentPlatform.name === 'windows' ? '.ps1' : '.sh'}`;

// Create build directory if it doesn't exist
if (!fs.existsSync(BUILD_DIR)) {
  fs.mkdirSync(BUILD_DIR, { recursive: true });
}

// Check if we need to run dependencies script
function checkDependencies() {
  console.log(chalk.blue('Checking dependencies...'));
  try {
    execSync('node scripts/deps.js', { stdio: 'inherit' });
  } catch (error) {
    console.error(chalk.red(`Failed to check dependencies: ${error.message}`));
    process.exit(1);
  }
}

// Generate resources
function generateResources() {
  if (IS_DEV) return; // Skip in dev mode
  
  console.log(chalk.blue('Generating resources from React build...'));
  
  try {
    // Change the working directory to the project root, not src-react
    if (currentPlatform.name === 'windows') {
      execSync(`powershell -File ${PLATFORM_GEN_SCRIPT}`, { 
        stdio: 'inherit',
        cwd: path.resolve(__dirname, '..')  // Project root
      });
    } else {
      execSync(`bash ${PLATFORM_GEN_SCRIPT}`, { 
        stdio: 'inherit',
        cwd: path.resolve(__dirname, '..')  // Project root
      });
    }
  } catch (error) {
    console.error(chalk.red(`Resource generation failed: ${error.message}`));
    process.exit(1);
  }
}

// Configure with CMake
function configureCMake() {
  console.log(chalk.blue(`Configuring CMake in ${BUILD_DIR}...`));
  
  try {
    execSync(
      `cmake -B ${BUILD_DIR} -DDEV_MODE=${IS_DEV ? 'ON' : 'OFF'} -DBINARY_NAME=${BINARY_NAME}`,
      { stdio: 'inherit' }
    );
  } catch (error) {
    console.error(chalk.red(`CMake configuration failed: ${error.message}`));
    process.exit(1);
  }
}

// Build with CMake
function buildCMake() {
  console.log(chalk.blue(`Building in ${BUILD_DIR}...`));
  
  try {
    execSync(
      `cmake --build ${BUILD_DIR} --parallel`,
      { stdio: 'inherit' }
    );
    console.log(chalk.green('Build completed successfully.'));
  } catch (error) {
    console.error(chalk.red(`Build failed: ${error.message}`));
    process.exit(1);
  }
}

// Start the application in dev mode
function startApp() {
  if (!IS_DEV || !WATCH_MODE) return;
  
  console.log(chalk.blue('Starting application...'));
  
  const appPath = `${BUILD_DIR}${currentPlatform.pathSeparator}${BINARY_NAME}${currentPlatform.execExtension}`;
  const useShell = currentPlatform.name !== 'windows';
  
  const appProcess = spawn(appPath, [], {
    stdio: 'inherit',
    shell: useShell
  });
  
  appProcess.on('close', (code) => {
    console.log(chalk.yellow(`Application exited with code ${code}`));
    process.exit(code);
  });
}

// Watch for changes in source files
function watchFiles() {
  if (!WATCH_MODE) return;
  
  console.log(chalk.blue('Watching for file changes...'));
  
  // List of directories to watch
  const dirsToWatch = ['lib', 'src'];
  
  let buildInProgress = false;
  let pendingBuild = false;
  
  dirsToWatch.forEach(dir => {
    fs.watch(dir, { recursive: true }, (eventType, filename) => {
      // Only rebuild on C++ file changes
      if (!filename || !filename.match(/\.(cpp|hpp|h|c)$/)) return;
      
      console.log(chalk.yellow(`File ${filename} changed, rebuilding...`));
      
      if (buildInProgress) {
        pendingBuild = true;
        return;
      }
      
      buildInProgress = true;
      
      try {
        buildCMake();
      } catch (error) {
        console.error(chalk.red(`Rebuild failed: ${error.message}`));
      } finally {
        buildInProgress = false;
        
        if (pendingBuild) {
          pendingBuild = false;
          process.nextTick(() => {
            console.log(chalk.yellow('Running pending build...'));
            try {
              buildCMake();
            } catch (error) {
              console.error(chalk.red(`Pending build failed: ${error.message}`));
            }
          });
        }
      }
    });
  });
}

// Main build process
async function main() {
  checkDependencies();
  generateResources();
  configureCMake();
  buildCMake();
  
  if (WATCH_MODE) {
    startApp();
    watchFiles();
  }
}

main().catch(error => {
  console.error(chalk.red(`Build script failed: ${error.message}`));
  process.exit(1);
});
