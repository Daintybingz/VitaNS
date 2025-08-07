#!/bin/bash

# 🏭 Ubuntu Mesa Rebuild + Copy Script
# Rebuilds Mesa on Ubuntu and prepares for Windows copy

set -e

echo "🏭 Starting Mesa rebuild on Ubuntu for Windows transfer..."

# Clean previous builds
echo "🧹 Cleaning previous builds..."
rm -rf build-switch build-production build-complete

# Create build directory
mkdir -p build-complete

# Configure with all necessary components
echo "⚙️  Configuring Mesa build..."
meson setup build-complete --cross-file switch.meson \
  -Dplatforms=surfaceless \
  -Dgallium-drivers=softpipe \
  -Dvulkan-drivers= \
  -Dglx=disabled \
  -Dgles1=disabled \
  -Dgles2=enabled \
  -Dopengl=true \
  -Dshared-glapi=disabled \
  -Dgbm=disabled \
  -Dtools= \
  -Dstrip=false \
  -Ddebug=false \
  -Doptimization=2

# Build all components
echo "🔨 Building Mesa..."
ninja -C build-complete

# Convert to regular archives
echo "📦 Converting to regular archives..."

# Use the existing working script
if [ -f "create_regular_archives.sh" ]; then
    # Update the script to use build-complete directory
    sed 's/build-switch/build-complete/g' create_regular_archives.sh > create_regular_archives_complete.sh
    chmod +x create_regular_archives_complete.sh
    ./create_regular_archives_complete.sh
else
    echo "⚠️  Warning: create_regular_archives.sh not found, skipping archive conversion"
fi

# Verify libraries
echo "🔍 Verifying libraries..."
ls -la build-complete/src/mapi/es2api/libGLESv2.a
ls -la build-complete/src/egl/libEGL.a
ls -la build-complete/src/util/libmesa_util.a

# Create copy package for Windows
echo "📦 Creating copy package for Windows..."
mkdir -p mesa-libraries-for-windows
cp build-complete/src/mapi/es2api/libGLESv2.a mesa-libraries-for-windows/
cp build-complete/src/mapi/glapi/libglapi_static.a mesa-libraries-for-windows/
cp build-complete/src/egl/libEGL.a mesa-libraries-for-windows/
cp build-complete/src/util/libmesa_util.a mesa-libraries-for-windows/
cp build-complete/src/gallium/drivers/softpipe/libsoftpipe.a mesa-libraries-for-windows/
cp build-complete/src/util/blake3/libblake3.a mesa-libraries-for-windows/
cp build-complete/src/mesa/libmesa.a mesa-libraries-for-windows/

# Copy headers if they exist
if [ -d "build-complete/include" ]; then
    cp -r build-complete/include mesa-libraries-for-windows/
fi

echo "✅ Mesa rebuild finished!"
echo "📊 Library sizes:"
ls -lh mesa-libraries-for-windows/*.a

echo ""
echo "🚀 NEXT STEPS FOR WINDOWS:"
echo "1. Copy mesa-libraries-for-windows/ to your Windows machine"
echo "2. Copy the .a files to Vita3K-Switch/external/custom-mesa/lib/"
echo "3. Copy the include/ folder to Vita3K-Switch/external/custom-mesa/include/"
echo "4. Commit and push to GitHub"
echo "5. GitHub Actions will use these pre-built libraries"
