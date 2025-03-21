#!/usr/bin/env node

const fs = require("fs");
const path = require("path");
const { execSync, spawn } = require("child_process");
const chalk = require("chalk");

// Directory and file paths
const SCRIPT_DIR = path.dirname(require.resolve("./create-icon.js"));
const ICON_PATH = path.resolve(SCRIPT_DIR, "icon.png");

// Check if icon already exists
if (fs.existsSync(ICON_PATH)) {
  console.log(chalk.green(`Icon already exists at: ${ICON_PATH}`));
  process.exit(0);
}

console.log(chalk.blue("Creating placeholder icon..."));

// Check if ImageMagick is available
function isImageMagickAvailable() {
  try {
    execSync("convert -version", { stdio: "ignore" });
    return true;
  } catch (error) {
    return false;
  }
}

if (isImageMagickAvailable()) {
  try {
    // Create a gradient background with an 'M' in the center
    execSync(
      `convert -size 256x256 gradient:#4A6CF7:#611F69 \
      -font Arial-Bold -pointsize 120 -gravity center \
      -stroke white -strokewidth 2 -annotate 0 "M" \
      -stroke none -fill white -annotate 0 "M" \
      "${ICON_PATH}"`,
      { stdio: "inherit" }
    );

    console.log(chalk.green(`✅ Icon created successfully: ${ICON_PATH}`));
  } catch (error) {
    console.error(chalk.red(`❌ Failed to create icon: ${error.message}`));
    createFallbackIcon();
  }
} else {
  console.warn(
    chalk.yellow("ImageMagick not found. Creating a simple fallback icon.")
  );
  createFallbackIcon();
}

// Create a fallback icon if ImageMagick is not available
function createFallbackIcon() {
  try {
    // Create a basic SVG icon
    const svgIcon = `<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg width="256" height="256" xmlns="http://www.w3.org/2000/svg">
  <defs>
    <linearGradient id="grad" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" style="stop-color:#4A6CF7;stop-opacity:1" />
      <stop offset="100%" style="stop-color:#611F69;stop-opacity:1" />
    </linearGradient>
  </defs>
  <rect width="256" height="256" fill="url(#grad)" />
  <text x="128" y="150" font-family="Arial" font-size="120" font-weight="bold" 
    text-anchor="middle" fill="white" stroke="white" stroke-width="2">M</text>
</svg>`;

    // Write the SVG file
    const svgPath = path.resolve(SCRIPT_DIR, "icon.svg");
    fs.writeFileSync(svgPath, svgIcon);

    // Try to convert SVG to PNG if rsvg-convert is available
    try {
      execSync(`rsvg-convert -w 256 -h 256 "${svgPath}" -o "${ICON_PATH}"`, {
        stdio: "ignore",
      });
      fs.unlinkSync(svgPath); // Remove the temporary SVG
      console.log(chalk.green(`✅ Icon created successfully: ${ICON_PATH}`));
    } catch (convertError) {
      // If conversion fails, keep the SVG and inform the user
      console.warn(
        chalk.yellow(
          `Unable to convert SVG to PNG. SVG icon created at: ${svgPath}`
        )
      );
      console.warn(
        chalk.yellow(
          `Please convert it to PNG manually and place it at: ${ICON_PATH}`
        )
      );

      // Create an empty PNG as a placeholder
      const emptyPNG = Buffer.from(
        "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==",
        "base64"
      );
      fs.writeFileSync(ICON_PATH, emptyPNG);
    }
  } catch (error) {
    console.error(
      chalk.red(`❌ Failed to create fallback icon: ${error.message}`)
    );
    process.exit(1);
  }
}
