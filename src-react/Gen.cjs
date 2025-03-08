const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const { execSync } = require('child_process');

// Configuration
const DIST_DIR = path.resolve(__dirname, './dist');
const OUTPUT_DIR = path.resolve(__dirname, '../res');
const HEADERS_DIR = path.join(OUTPUT_DIR, 'headers');
const MAIN_HEADER = path.join(OUTPUT_DIR, 'aggregate.hpp');
const HASH_FILE = path.join(OUTPUT_DIR, 'resource_hashes.txt');
const TEMP_FOLDER = path.join(OUTPUT_DIR, 'tmp');  // Windows-friendly temp folder
const TEMP_RESOURCES = path.join(TEMP_FOLDER, 'membrane_resources.txt');

// Ensure necessary directories exist
if (fs.existsSync(OUTPUT_DIR)) {
    fs.rmSync(OUTPUT_DIR, { recursive: true, force: true });
}
fs.mkdirSync(OUTPUT_DIR, { recursive: true });
fs.mkdirSync(HEADERS_DIR, { recursive: true });
fs.mkdirSync(TEMP_FOLDER, { recursive: true });  // Ensure temp folder exists ✅

console.log(`✅ Temp folder created at: ${TEMP_FOLDER}`);

// Load previous hashes
const prevHashes = new Map();
if (fs.existsSync(HASH_FILE)) {
    const hashData = fs.readFileSync(HASH_FILE, 'utf-8').split('\n');
    hashData.forEach(line => {
        const [hash, ...fileParts] = line.split(' ');
        if (hash && fileParts.length) {
            prevHashes.set(fileParts.join(' '), hash);
        }
    });
}

// Empty temp files
fs.writeFileSync(TEMP_RESOURCES, '');
fs.writeFileSync(MAIN_HEADER, '');
fs.writeFileSync(HASH_FILE, '');

// Function to create a valid C variable name from file path
function makeVarName(file) {
    const relPath = path.relative(DIST_DIR, file);
    return `res_${relPath.replace(/[^a-zA-Z0-9_]/g, '_')}`;
}

// Add header guard to the main header file
fs.writeFileSync(MAIN_HEADER, `#ifndef MEMBRANE_RESOURCES_HPP\n#define MEMBRANE_RESOURCES_HPP\n// Auto-generated resource headers for Membrane\n`);

console.log(`Processing files in ${DIST_DIR}...`);

// Function to process files
function processFile(file) {
    const relPath = path.relative(DIST_DIR, file);
    const varName = makeVarName(file);
    const fileContent = fs.readFileSync(file);
    const newHash = crypto.createHash('sha256').update(fileContent).digest('hex');

    if (prevHashes.get(relPath) === newHash) {
        console.log(`Skipping unchanged: ${relPath}`);
    } else {
        console.log(`Processing: ${relPath}`);
        const headerPath = path.join(HEADERS_DIR, `${varName}.hpp`);
        execSync(`xxd -i -n ${varName} "${file}" > "${headerPath}"`);
    }

    fs.appendFileSync(HASH_FILE, `${newHash} ${relPath}\n`);
    fs.appendFileSync(MAIN_HEADER, `#include "headers/${varName}.hpp"\n`);
    fs.appendFileSync(TEMP_RESOURCES, `${varName}|${relPath}\n`);
}

// Recursive function to scan directories
function scanDir(dir) {
    fs.readdirSync(dir, { withFileTypes: true }).forEach(entry => {
        const fullPath = path.join(dir, entry.name);
        if (entry.isDirectory()) {
            scanDir(fullPath);
        } else {
            processFile(fullPath);
        }
    });
}

scanDir(DIST_DIR);

fs.appendFileSync(MAIN_HEADER, "#endif\n");

// Create a resource initialization file
const resourceInitCpp = path.join(OUTPUT_DIR, 'resource_init.cpp');
const resourceInitHpp = path.join(OUTPUT_DIR, 'resource_init.hpp');

fs.writeFileSync(resourceInitCpp, `#include "Membrane.hpp"\n#include "aggregate.hpp"\nvoid initialize_resources(Membrane& app) {\n`);

const tempResourcesData = fs.readFileSync(TEMP_RESOURCES, 'utf-8').split('\n').filter(line => line);
tempResourcesData.forEach(line => {
    const [varName, filePath] = line.split('|');
    if (varName && filePath) {
        fs.appendFileSync(resourceInitCpp, `    app.add_vfs("${filePath}", ${varName}, ${varName}_len);\n`);
    }
});

fs.appendFileSync(resourceInitCpp, `}\n`);

fs.writeFileSync(resourceInitHpp, `#ifndef MEMBRANE_RESOURCE_INIT_HPP\n#define MEMBRANE_RESOURCE_INIT_HPP\n\nclass Membrane;\nvoid initialize_resources(Membrane& app);\n\n#endif // MEMBRANE_RESOURCE_INIT_HPP\n`);

// Cleanup temp file
fs.unlinkSync(TEMP_RESOURCES);

console.log('✅ Resource compilation complete!');
