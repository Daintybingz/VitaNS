#!/bin/bash
set -e

echo "📦 Copying Mesa libraries to Vita3K-Switch custom-mesa directory..."

# Check if Mesa build directory exists
if [ ! -d "Mesa/build-switch" ]; then
    echo "❌ Error: Mesa/build-switch directory not found. Run Mesa build first."
    exit 1
fi

# Create custom-mesa directories if they don't exist
mkdir -p Vita3K-Switch/external/custom-mesa/lib
mkdir -p Vita3K-Switch/external/custom-mesa/include

echo "🔍 Copying libraries from Mesa/build-switch to Vita3K-Switch/external/custom-mesa/lib/"

# Copy all Mesa libraries
cp Mesa/build-switch/src/mapi/es2api/libGLESv2.a Vita3K-Switch/external/custom-mesa/lib/
cp Mesa/build-switch/src/egl/libEGL.a Vita3K-Switch/external/custom-mesa/lib/
cp Mesa/build-switch/src/util/libmesa_util.a Vita3K-Switch/external/custom-mesa/lib/
cp Mesa/build-switch/src/gallium/drivers/softpipe/libsoftpipe.a Vita3K-Switch/external/custom-mesa/lib/

echo "📁 Copying headers from Mesa source to Vita3K-Switch/external/custom-mesa/include/"

# Copy Mesa headers from source
cp -r Mesa/include/* Vita3K-Switch/external/custom-mesa/include/

echo "✅ Libraries copied successfully!"
echo ""
echo "📊 Copied libraries:"
ls -la Vita3K-Switch/external/custom-mesa/lib/

echo ""
echo "📁 Copied headers:"
ls -la Vita3K-Switch/external/custom-mesa/include/

echo ""
echo "🎉 Mesa libraries are now ready for Vita3K-Switch build!"
