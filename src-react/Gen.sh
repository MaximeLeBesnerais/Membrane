#!/bin/bash

# Membrane Resource Compiler
# Transforms React build output into C++ headers for Membrane
# Uses hashing to prevent unnecessary reprocessing

# Configuration
DIST_DIR="./dist"
OUTPUT_DIR="../res"
HEADERS_DIR="$OUTPUT_DIR/headers"
MAIN_HEADER="aggregate.hpp"
HASH_FILE="../res/resource_hashes.txt"
TEMP_RESOURCES="/tmp/membrane_resources.txt"

if [ -d "$OUTPUT_DIR" ]; then
    rm -r "$OUTPUT_DIR"
fi

mkdir -p "$OUTPUT_DIR"
mkdir -p "$HEADERS_DIR"

# Load previous hashes
declare -A prev_hashes
if [[ -f "$HASH_FILE" ]]; then
    while IFS=" " read -r hash file; do
        prev_hashes["$file"]="$hash"
    done < "$HASH_FILE"
fi

> "$TEMP_RESOURCES"
> "$OUTPUT_DIR/$MAIN_HEADER"
> "$HASH_FILE"

# Function to create a valid C variable name from file path
function make_var_name() {
    local file="$1"
    local rel_path="${file#$DIST_DIR/}"
    local var_name="res_$(echo "$rel_path" | tr -c '[:alnum:]' '_')"
    echo "$var_name"
}

# Add header guard to the main header file
cat > "$OUTPUT_DIR/$MAIN_HEADER" << EOF
#ifndef MEMBRANE_RESOURCES_HPP
#define MEMBRANE_RESOURCES_HPP
// Auto-generated resource headers for Membrane
EOF

echo "Processing files in $DIST_DIR..."

find "$DIST_DIR" -type f -not -path "*/\.*" | while read -r file; do
    rel_path="${file#$DIST_DIR/}"
    var_name=$(make_var_name "$file")
    new_hash=$(sha256sum "$file" | awk '{print $1}')

    if [[ "${prev_hashes[$rel_path]}" == "$new_hash" ]]; then
        echo "Skipping unchanged: $rel_path"
    else
        echo "Processing: $rel_path"
        xxd -i -n "$var_name" "$file" > "$HEADERS_DIR/${var_name}.hpp"
    fi

    echo "$new_hash $rel_path" >> "$HASH_FILE"
    echo "#include \"headers/${var_name}.hpp\"" >> "$OUTPUT_DIR/$MAIN_HEADER"
    echo "$var_name|$rel_path" >> "$TEMP_RESOURCES"
done

echo "#endif" >> "$OUTPUT_DIR/$MAIN_HEADER"

# Create a resource initialization file
cat > "$OUTPUT_DIR/resource_init.cpp" << EOF
#include "Membrane.hpp"
#include "aggregate.hpp"
void initialize_resources(Membrane& app) {
$(while IFS="|" read -r var_name path; do
    echo "    app.add_vfs(\"$path\", $var_name, ${var_name}_len);"
done < "$TEMP_RESOURCES")
}
EOF

# Create a header for the initialization function
cat > "$OUTPUT_DIR/resource_init.hpp" << EOF
#ifndef MEMBRANE_RESOURCE_INIT_HPP
#define MEMBRANE_RESOURCE_INIT_HPP

class Membrane;
void initialize_resources(Membrane& app);

#endif // MEMBRANE_RESOURCE_INIT_HPP
EOF

rm "$TEMP_RESOURCES"
echo "✅ Resource compilation complete!"
