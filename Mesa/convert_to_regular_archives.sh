#!/bin/bash

echo "🔧 Converting thin archives to regular archives..."
echo "================================================"

# Check if build directory exists
if [ ! -d "build-switch" ]; then
    echo "❌ Error: build-switch directory not found. Run build_real_mesa.sh first."
    exit 1
fi

# Function to convert thin archive to regular archive
convert_archive() {
    local archive_path="$1"
    local archive_name=$(basename "$archive_path")
    local archive_dir=$(dirname "$archive_path")
    
    echo "Converting $archive_name..."
    
    # Create temporary directory for extraction
    local temp_dir="/tmp/mesa_convert_$$"
    mkdir -p "$temp_dir"
    
    # Use host ar to extract all object files from thin archive
    cd "$temp_dir"
    ar x "$archive_path"
    
    # Check if extraction was successful
    if [ $? -eq 0 ] && [ -n "$(ls *.o 2>/dev/null)" ]; then
        # Use devkitPro ar to create new regular archive with all object files
        /opt/devkitpro/devkitA64/bin/aarch64-none-elf-ar rcs "$archive_path" *.o
        
        if [ $? -eq 0 ]; then
            echo "  ✅ Successfully converted $archive_name"
        else
            echo "  ❌ Failed to create regular archive for $archive_name"
        fi
    else
        echo "  ❌ Failed to extract object files from $archive_name"
    fi
    
    # Clean up
    cd - > /dev/null
    rm -rf "$temp_dir"
}

# Find all thin archives and convert them
echo "🔍 Finding thin archives..."
thin_archives=()

# Use absolute paths
while IFS= read -r file; do
    if file "$file" | grep -q "thin archive"; then
        thin_archives+=("$file")
    fi
done < <(find "$(pwd)/build-switch" -name "*.a")

echo "Found ${#thin_archives[@]} thin archives to convert:"

for archive in "${thin_archives[@]}"; do
    echo "  - $(basename "$archive")"
done

echo ""
echo "🔄 Converting archives..."

for archive in "${thin_archives[@]}"; do
    convert_archive "$archive"
done

echo ""
echo "🔍 Verifying conversion..."

# Check final archive types
echo "Final archive types:"
find build-switch -name "*.a" -exec file {} \; | sort

echo ""
echo "✅ Archive conversion completed!"
echo "All libraries should now be regular archives compatible with devkitPro." 