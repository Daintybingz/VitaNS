#!/bin/bash

echo "🔧 Creating regular archives from object files..."
echo "================================================"

# Check if build directory exists
if [ ! -d "build-production" ]; then
    echo "❌ Error: build-production directory not found. Run build_real_mesa.sh first."
    exit 1
fi

# Function to create regular archive from object files
create_regular_archive() {
    local archive_path="$1"
    local archive_name=$(basename "$archive_path")
    local archive_dir=$(dirname "$archive_path")
    
    echo "Creating regular archive for $archive_name..."
    
    # Remove existing thin archive if it exists
    if [ -f "$archive_path" ]; then
        echo "  Removing existing thin archive..."
        rm "$archive_path"
    fi
    
    # Find all object files in the build directory that belong to this library
    # We'll look for object files in the same directory structure
    local obj_files=()
    
    # Get the relative path from build-production
    local rel_path=$(echo "$archive_path" | sed 's|build-production/||')
    local lib_dir=$(dirname "$rel_path")
    
    # Find object files that might belong to this library
    if [ -d "build-production/$lib_dir" ]; then
        # Look for .o files in the same directory
        while IFS= read -r obj_file; do
            obj_files+=("$obj_file")
        done < <(find "build-production/$lib_dir" -name "*.o" 2>/dev/null)
    fi
    
    # Also look for object files in subdirectories that might belong to this library
    if [ ${#obj_files[@]} -eq 0 ]; then
        # Try to find object files by looking at the library name
        local lib_base=$(echo "$archive_name" | sed 's|lib||' | sed 's|\.a||')
        while IFS= read -r obj_file; do
            obj_files+=("$obj_file")
        done < <(find build-production -name "*.o" -path "*$lib_base*" 2>/dev/null)
    fi
    
    if [ ${#obj_files[@]} -gt 0 ]; then
        echo "  Found ${#obj_files[@]} object files for $archive_name"
        
        # Create new regular archive with all object files
        /opt/devkitpro/devkitA64/bin/aarch64-none-elf-ar rcs "$archive_path" "${obj_files[@]}"
        
        if [ $? -eq 0 ]; then
            echo "  ✅ Successfully created regular archive for $archive_name"
        else
            echo "  ❌ Failed to create regular archive for $archive_name"
        fi
    else
        echo "  ❌ No object files found for $archive_name"
    fi
}

# List of libraries to convert
libraries=(
    "build-production/src/mapi/es2api/libGLESv2.a"
    "build-production/src/mapi/glapi/libglapi_static.a"
    "build-production/src/util/libmesa_util.a"
    "build-production/src/gallium/drivers/softpipe/libsoftpipe.a"
    "build-production/src/util/blake3/libblake3.a"
    "build-production/src/mesa/libmesa.a"
)

echo "🔍 Libraries to convert:"
for lib in "${libraries[@]}"; do
    echo "  - $(basename "$lib")"
done

echo ""
echo "🔄 Creating regular archives..."

for lib in "${libraries[@]}"; do
    if [ -f "$lib" ] || [ -d "$(dirname "$lib")" ]; then
        create_regular_archive "$lib"
    else
        echo "❌ Library directory not found: $(dirname "$lib")"
    fi
done

echo ""
echo "🔍 Verifying conversion..."

# Check final archive types
echo "Final archive types:"
find build-production -name "*.a" -exec file {} \; | sort

echo ""
echo "✅ Regular archive creation completed!"
echo "All libraries should now be regular archives compatible with devkitPro." 