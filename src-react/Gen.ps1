# Membrane Resource Compiler for Windows
# Transforms React build output into C++ headers for Membrane

# Print banner
Write-Host "­ƒöº Membrane Resource Compiler for Windows"
Write-Host "Current working directory: `"$((Get-Location).Path)`""

# Configuration
$DIST_DIR = "./dist"
$OUTPUT_DIR = "../res"
$HEADERS_DIR = "$OUTPUT_DIR/headers"
$MAIN_HEADER = "aggregate.hpp"
$HASH_FILE = "$OUTPUT_DIR/resource_hashes.txt"
$TEMP_RESOURCES = "$env:TEMP/membrane_resources.txt"

# Ensure output directories exist
if (Test-Path $OUTPUT_DIR) {
    Remove-Item -Path $OUTPUT_DIR -Recurse -Force
}

New-Item -ItemType Directory -Path $OUTPUT_DIR -Force | Out-Null
New-Item -ItemType Directory -Path $HEADERS_DIR -Force | Out-Null

# Load previous hashes
$prev_hashes = @{}
if (Test-Path $HASH_FILE) {
    Get-Content $HASH_FILE | ForEach-Object {
        $hash, $file = $_ -split ' ', 2
        $prev_hashes[$file] = $hash
    }
}

# Clear temporary and output files
"" | Set-Content $TEMP_RESOURCES
"" | Set-Content "$OUTPUT_DIR/$MAIN_HEADER"
"" | Set-Content $HASH_FILE

# Function to create a valid C variable name from file path
function Make-VarName {
    param($file)
    $rel_path = $file -replace [regex]::Escape($DIST_DIR + "/"), ""
    $var_name = "res_" + ($rel_path -replace '[^a-zA-Z0-9]', '_')
    return $var_name
}

# Add header guard to the main header file
@"
#ifndef MEMBRANE_RESOURCES_HPP
#define MEMBRANE_RESOURCES_HPP
// Auto-generated resource headers for Membrane
"@ | Set-Content "$OUTPUT_DIR/$MAIN_HEADER"

Write-Host "Processing files in $DIST_DIR..."

# Process each file in the distribution directory
Get-ChildItem -Path $DIST_DIR -File -Recurse | ForEach-Object {
    $file = $_.FullName
    $rel_path = $file -replace [regex]::Escape((Get-Location).Path + "\$DIST_DIR\"), ""
    $rel_path = $rel_path -replace "\\", "/"
    
    Write-Host "Checking: $file"
    Write-Host "File name: $($_.Name)"
    Write-Host "Resource path: $rel_path"
    
    $var_name = Make-VarName $rel_path
    Write-Host "Variable name: $var_name"
    
    # Calculate hash of the file
    $new_hash = (Get-FileHash -Path $file -Algorithm SHA256).Hash
    
    if ($prev_hashes[$rel_path] -eq $new_hash) {
        Write-Host "Skipping unchanged: $rel_path"
    } else {
        Write-Host "Processing: $rel_path"
        
        # Generate C header using xxd-like functionality
        $bytes = [System.IO.File]::ReadAllBytes($file)
        $output = "unsigned char $var_name[] = {`n"
        $lineCount = 0
        
        for ($i = 0; $i -lt $bytes.Length; $i++) {
            if ($lineCount -eq 0) {
                $output += "  "
            }
            
            $output += "0x" + $bytes[$i].ToString("x2") + ", "
            $lineCount++
            
            if ($lineCount -eq 12) {
                $output += "`n"
                $lineCount = 0
            }
        }
        
        if ($lineCount -ne 0) {
            $output += "`n"
        }
        
        $output += "};`nunsigned int ${var_name}_len = " + $bytes.Length + ";`n"
        $output | Set-Content "$HEADERS_DIR/${var_name}.hpp"
    }
    
    # Save hash and update includes
    "$new_hash $rel_path" | Add-Content $HASH_FILE
    "#include `"headers/${var_name}.hpp`"" | Add-Content "$OUTPUT_DIR/$MAIN_HEADER"
    "$var_name|$rel_path" | Add-Content $TEMP_RESOURCES
}

# Close the main header
"#endif" | Add-Content "$OUTPUT_DIR/$MAIN_HEADER"

# Create a resource initialization file
$init_content = @"
#include "Membrane.hpp"
#include "aggregate.hpp"
void initialize_resources(Membrane& app) {

"@

# Add each resource to the initialization file
Get-Content $TEMP_RESOURCES | ForEach-Object {
    $var_name, $path = $_ -split '\|', 2
    $init_content += "    app.add_vfs(""$path"", $var_name, ${var_name}_len);`n"
}

$init_content += "}`n"
$init_content | Set-Content "$OUTPUT_DIR/resource_init.cpp"

# Create a header for the initialization function
@"
#ifndef MEMBRANE_RESOURCE_INIT_HPP
#define MEMBRANE_RESOURCE_INIT_HPP

class Membrane;
void initialize_resources(Membrane& app);

#endif // MEMBRANE_RESOURCE_INIT_HPP
"@ | Set-Content "$OUTPUT_DIR/resource_init.hpp"

# Clean up
Remove-Item $TEMP_RESOURCES -Force

Write-Host "Resource compilation complete!"