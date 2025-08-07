#!/bin/bash
set -e

echo "Converting thin archives to regular archives..."

# Function to convert thin archive to regular archive
convert_archive() {
    local archive="$1"
    if [ -f "$archive" ]; then
        echo "Converting $archive..."
        
        # Create temporary directory
        temp_dir=$(mktemp -d)
        
        # Extract objects from thin archive
        ar x "$archive"
        
        # Create new regular archive
        ar rcs "${archive}.new" *.o
        
        # Replace thin archive with regular archive
        mv "${archive}.new" "$archive"
        
        # Clean up
        rm -f *.o
        rmdir "$temp_dir" 2>/dev/null || true
    fi
}

# Convert all libraries
convert_archive "build-complete/src/mapi/es2api/libGLESv2.a"
convert_archive "build-complete/src/mapi/glapi/libglapi_static.a"
convert_archive "build-complete/src/egl/libEGL.a"
convert_archive "build-complete/src/util/libmesa_util.a"
convert_archive "build-complete/src/gallium/drivers/softpipe/libsoftpipe.a"
convert_archive "build-complete/src/util/blake3/libblake3.a"
convert_archive "build-complete/src/mesa/libmesa.a"

echo "✅ Archive conversion complete"
