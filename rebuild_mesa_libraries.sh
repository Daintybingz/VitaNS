#!/bin/bash

# Rebuild Mesa libraries for VitaNS Switch port
# This script rebuilds Mesa with regular static archives (not thin archives)

set -e

echo "🎮 Rebuilding Mesa libraries for VitaNS Switch port..."
echo "📦 Ensuring regular static archives (not thin archives)"

# Go to Mesa directory
cd Mesa

# Clean previous build
echo "🧹 Cleaning previous build..."
rm -rf build-switch

# Configure build with regular static archives
echo "⚙️  Configuring build..."
meson setup build-switch --cross-file switch.meson \
  -Dgles2=enabled \
  -Degl=enabled \
  -Dgallium=enabled \
  -Dshared-glapi=disabled \
  -Dglx=disabled \
  -Dosmesa=disabled \
  -Ddri=disabled \
  -Ddri3=disabled \
  -Dgbm=disabled \
  -Dxlib-lease=disabled \
  -Dvideo-codecs=disabled \
  -Dvulkan-beta=disabled \
  -Dvulkan-layers=disabled \
  -Dbuild-tests=false \
  -Dbuild-aco-tests=false \
  -Dglcpp-tests=false \
  -Ddefault-library=static

# Set environment to force regular archives
export ARFLAGS="-crs"
export AR="aarch64-none-elf-ar -crs"

echo "🔨 Building libraries..."
cd build-switch

# Build with explicit archive flags
ninja -v

echo "✅ Build completed!"

# Verify archives are regular (not thin)
echo "🔍 Verifying archive types..."
echo "Checking for thin archives (should find none):"

# Check if any archives are thin
thin_archives=$(find . -name "*.a" -exec file {} \; | grep "thin archive" || true)
if [ -n "$thin_archives" ]; then
    echo "❌ WARNING: Found thin archives:"
    echo "$thin_archives"
    echo "This may cause linking issues!"
else
    echo "✅ All archives are regular static archives (good!)"
fi

# Copy libraries to VitaNS
echo "📁 Copying libraries to VitaNS..."
OUTPUT_DIR="../Vita3K-Switch/external/custom-mesa/lib"

# Ensure output directory exists
mkdir -p "$OUTPUT_DIR"

# Copy all .a files
find . -name "*.a" -exec cp {} "$OUTPUT_DIR" \;

echo "✅ Libraries copied to $OUTPUT_DIR"
ls -la "$OUTPUT_DIR"

# Copy headers
echo "📁 Copying headers to VitaNS..."
HEADER_DIR="../Vita3K-Switch/external/custom-mesa/include"

# Ensure header directory exists
mkdir -p "$HEADER_DIR"

# Copy EGL headers
if [ -d "../include/EGL" ]; then
    cp -r ../include/EGL "$HEADER_DIR/"
fi

# Copy GLES2 headers
if [ -d "../include/GLES2" ]; then
    cp -r ../include/GLES2 "$HEADER_DIR/"
fi

# Copy KHR headers
if [ -d "../include/KHR" ]; then
    cp -r ../include/KHR "$HEADER_DIR/"
fi

# Copy root headers
cp ../include/*.h "$HEADER_DIR/" 2>/dev/null || true

echo "✅ Headers copied to $HEADER_DIR"
ls -la "$HEADER_DIR" || echo "No headers found"

echo ""
echo "🎉 Mesa rebuild completed successfully!"
echo "📁 Libraries and headers are in Vita3K-Switch/external/custom-mesa/"
echo "🔗 Ready for linking in VitaNS!" 