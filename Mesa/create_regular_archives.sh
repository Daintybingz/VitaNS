#!/bin/bash

echo "🔨 Creating regular archives from object files..."

# Get the current directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Function to create regular archive from object files
create_archive() {
    local archive_path="$1"
    local object_pattern="$2"
    local archive_name=$(basename "$archive_path")
    local archive_dir=$(dirname "$archive_path")
    
    echo "Creating $archive_name..."
    
    # Remove existing archive if it exists
    if [ -f "$archive_path" ]; then
        echo "Removing existing $archive_name..."
        rm "$archive_path"
    fi
    
    # Find all object files for this library
    local object_files=$(find "${SCRIPT_DIR}/build-switch" -path "$object_pattern" -name "*.o" 2>/dev/null)
    
    if [ -z "$object_files" ]; then
        echo "❌ No object files found for $archive_name"
        return 1
    fi
    
    echo "Found $(echo "$object_files" | wc -l) object files"
    
    # Create the archive
    ar rcs "$archive_path" $object_files
    
    if [ $? -eq 0 ]; then
        echo "✅ Created $archive_name"
    else
        echo "❌ Failed to create $archive_name"
        return 1
    fi
}

# Create regular archives from object files
create_archive "${SCRIPT_DIR}/build-switch/src/util/libmesa_util.a" "*/libmesa_util.a.p/*"
create_archive "${SCRIPT_DIR}/build-switch/src/mesa/libmesa.a" "*/libmesa.a.p/*"
create_archive "${SCRIPT_DIR}/build-switch/src/mapi/es2api/libGLESv2.a" "*/libGLESv2.a.p/*"
create_archive "${SCRIPT_DIR}/build-switch/src/util/blake3/libblake3.a" "*/libblake3.a.p/*"
create_archive "${SCRIPT_DIR}/build-switch/src/gallium/drivers/softpipe/libsoftpipe.a" "*/libsoftpipe.a.p/*"

echo "✅ All archives created!"
echo "📊 Final archive types:"
file "${SCRIPT_DIR}/build-switch/src/util/libmesa_util.a" "${SCRIPT_DIR}/build-switch/src/mesa/libmesa.a" "${SCRIPT_DIR}/build-switch/src/egl/libEGL.a" "${SCRIPT_DIR}/build-switch/src/mapi/es2api/libGLESv2.a" "${SCRIPT_DIR}/build-switch/src/util/blake3/libblake3.a" "${SCRIPT_DIR}/build-switch/src/gallium/drivers/softpipe/libsoftpipe.a"

echo "📦 Archive sizes:"
ls -lh "${SCRIPT_DIR}/build-switch/src/util/libmesa_util.a" "${SCRIPT_DIR}/build-switch/src/mesa/libmesa.a" "${SCRIPT_DIR}/build-switch/src/egl/libEGL.a" "${SCRIPT_DIR}/build-switch/src/mapi/es2api/libGLESv2.a" "${SCRIPT_DIR}/build-switch/src/util/blake3/libblake3.a" "${SCRIPT_DIR}/build-switch/src/gallium/drivers/softpipe/libsoftpipe.a" 