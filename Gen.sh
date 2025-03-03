#!/bin/bash

# Membrane Resource Compiler
# Transforms React build output into C++ headers for Membrane
# With subdirectory organization for cleaner structure

# Configuration
DIST_DIR="./dist"
OUTPUT_DIR="../res"
HEADERS_DIR="$OUTPUT_DIR/headers"
MAIN_HEADER="aggregate.hpp"
TEMP_RESOURCES="/tmp/membrane_resources.txt"

# Create output directories if they don't exist
mkdir -p "$OUTPUT_DIR"
mkdir -p "$HEADERS_DIR"

# Clear the temporary file and main header
> "$TEMP_RESOURCES"
> "$OUTPUT_DIR/$MAIN_HEADER"

# Function to create a valid C variable name from file path
function make_var_name() {
    local file="$1"
    local rel_path="${file#$DIST_DIR/}"
    # Replace non-alphanumeric characters with underscore
    local var_name="res_$(echo "$rel_path" | tr -c '[:alnum:]' '_')"
    echo "$var_name"
}

# Add header guard to the main header file
cat > "$OUTPUT_DIR/$MAIN_HEADER" << EOF
#ifndef MEMBRANE_RESOURCES_HPP
#define MEMBRANE_RESOURCES_HPP

// Auto-generated resource headers for Membrane
// Generated on $(date)

EOF

echo "Processing files in $DIST_DIR..."

# Find and process each file in the dist directory
find "$DIST_DIR" -type f -not -path "*/\.*" | while read -r file; do
    # Skip directories
    if [ -d "$file" ]; then
        continue
    fi

    # Get relative path from dist directory
    rel_path="${file#$DIST_DIR/}"

    # Create variable name
    var_name=$(make_var_name "$file")

    # Create a header file for this resource in the subdirectory
    xxd -i -n "$var_name" "$file" > "$HEADERS_DIR/${var_name}.hpp"

    # Add an include statement to the main header
    echo "#include \"headers/${var_name}.hpp\"" >> "$OUTPUT_DIR/$MAIN_HEADER"

    # Store resource information for initialization code
    echo "$var_name|$rel_path" >> "$TEMP_RESOURCES"

    echo "Processed: $rel_path -> headers/${var_name}.hpp"
done

# Close the header guard in the main header
echo "" >> "$OUTPUT_DIR/$MAIN_HEADER"
echo "#endif // MEMBRANE_RESOURCES_HPP" >> "$OUTPUT_DIR/$MAIN_HEADER"

# Create a resource initialization file
cat > "$OUTPUT_DIR/resource_init.cpp" << EOF
// Auto-generated resource initialization for Membrane
// Generated on $(date)

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
// Auto-generated resource initialization header for Membrane
// Generated on $(date)

#ifndef MEMBRANE_RESOURCE_INIT_HPP
#define MEMBRANE_RESOURCE_INIT_HPP

class Membrane;

// Initialize all resources for the given Membrane app instance
void initialize_resources(Membrane& app);

#endif // MEMBRANE_RESOURCE_INIT_HPP
EOF

# Cleanup temporary files
rm "$TEMP_RESOURCES"

echo "Resource compilation complete!"
echo "Main header: $OUTPUT_DIR/$MAIN_HEADER"
echo "Individual headers: $HEADERS_DIR/"
echo "Initialization function: $OUTPUT_DIR/resource_init.cpp"
echo ""
echo "Usage in main.cpp:"
echo "  #include \"resource_init.hpp\""
echo "  initialize_resources(app);"
echo ""
echo "Remember to include the headers in your source files!"
