#!/usr/bin/env node

const fs = require('fs');
const path = require('path');
const rimraf = require('rimraf');
const chalk = require('chalk');

// Directories to clean
const DIRS_TO_CLEAN = [
  'build',
  'build-dev',
  'res'
];

// Clean build directories
console.log(chalk.blue('Cleaning build directories...'));

DIRS_TO_CLEAN.forEach(dir => {
  const fullPath = path.join(__dirname, '..', dir);
  
  if (fs.existsSync(fullPath)) {
    console.log(chalk.yellow(`Removing ${dir}...`));
    rimraf.sync(fullPath);
  }
});

// Clean node_modules
if (process.argv.includes('--deep')) {
  console.log(chalk.blue('Performing deep clean...'));
  
  // Clean root node_modules
  const rootNodeModules = path.join(__dirname, '..', 'node_modules');
  if (fs.existsSync(rootNodeModules)) {
    console.log(chalk.yellow('Removing root node_modules...'));
    rimraf.sync(rootNodeModules);
  }
  
  // Clean React node_modules
  const reactNodeModules = path.join(__dirname, '..', 'src-react', 'node_modules');
  if (fs.existsSync(reactNodeModules)) {
    console.log(chalk.yellow('Removing React node_modules...'));
    rimraf.sync(reactNodeModules);
  }
}

console.log(chalk.green('Clean complete!'));
