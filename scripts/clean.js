#!/usr/bin/env node

const fs = require("fs");
const path = require("path");
const rimraf = require("rimraf");
const chalk = require("chalk");

// Directories to clean
const DIRS_TO_CLEAN = ["build", "build-dev", "res", "deps"];

// Clean build directories
console.log(chalk.blue("Cleaning build directories..."));

DIRS_TO_CLEAN.forEach((dir) => {
  const fullPath = path.join(__dirname, "..", dir);

  if (fs.existsSync(fullPath)) {
    console.log(chalk.yellow(`Removing ${dir}...`));
    rimraf.sync(fullPath);
  }
});

console.log(chalk.blue("Cleaning React dist, hash..."));

const reactDist = path.join(__dirname, "..", "src-react", "dist");
const reactHash = path.join(__dirname, "..", "src-react", "build-hash.txt");
if (fs.existsSync(reactDist)) {
  console.log(chalk.yellow("Removing React dist..."));
  rimraf.sync(reactDist);
  console.log(chalk.yellow("Removing React hash..."));
  fs.existsSync(reactHash) && rimraf.sync(reactHash);
}

// Clean node_modules
if (process.argv.includes("--deep")) {
  console.log(chalk.blue("Performing deep clean..."));

  // remove .cache at root
  const cacheDir = path.join(__dirname, "..", ".cache");
  if (fs.existsSync(cacheDir)) {
    console.log(chalk.yellow("Removing .cache..."));
    rimraf.sync(cacheDir);
  }

  // Clean React node_modules
  const reactNodeModules = path.join(
    __dirname,
    "..",
    "src-react",
    "node_modules"
  );
  if (fs.existsSync(reactNodeModules)) {
    console.log(chalk.yellow("Removing React node_modules..."));
    rimraf.sync(reactNodeModules);
  }

  // Clean root node_modules
  const rootNodeModules = path.join(__dirname, "..", "node_modules");
  if (fs.existsSync(rootNodeModules)) {
    console.log(chalk.yellow("Removing root node_modules..."));
    rimraf.sync(rootNodeModules);
  }
}

console.log(chalk.green("Clean complete!"));
