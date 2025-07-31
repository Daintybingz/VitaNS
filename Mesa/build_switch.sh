#!/bin/bash

# Mesa Nintendo Switch Build Script
# Updated to ensure regular static archives (not thin archives)

set -e

echo "🎮 Building Mesa for Nintendo Switch..."
echo "📦 Ensuring regular static archives (not thin archives)"

# Clean previous build
if [ -d "build-switch" ]; then
    echo "🧹 Cleaning previous build..."
    rm -rf build-switch
fi

# Configure build with regular static archives
echo "⚙️  Configuring build..."
meson setup build-switch --cross-file switch.meson \
  -Dgles2=enabled \
  -Degl=enabled \
  -Dgallium-drivers=swrast \
  -Dglx=disabled \
  -Dgbm=disabled \
  -Dllvm=disabled \
  -Dvulkan-drivers=[] \
  -Dgallium-opencl=disabled \
  -Dgallium-va=disabled \
  -Dgallium-vdpau=disabled \
  -Dgallium-xa=disabled \
  -Dgallium-nine=false \
  -Dgallium-omx=disabled \
  -Dtools=[] \
  -Dplatforms=surfaceless \
  -Dopengl=false \
  -Dgles1=disabled \
  -Dshader-cache=disabled \
  -Dzlib=disabled \
  -Dzstd=disabled \
  -Dexpat=disabled \
  -Dvalgrind=disabled \
  -Dlmsensors=disabled \
  -Dselinux=disabled \
  -Dlibunwind=disabled \
  -Dandroid-stub=false \
  -Dandroid-strict=false \
  -Dgallium-rusticl=false \
  -Dgallium-d3d10umd=false \
  -Dpower8=false \
  -Dvmware-mks-stats=false \
  -Dcpp-rtti=false \
  -Dallow-kcmp=false \
  -Dmesa-debug=false \
  -Dshader-cache-default=false \
  -Dsse2=false \
  -Ddraw-use-llvm=false \
  -Dshared-llvm=false \
  -Dopencl-spirv=false \
  -Dgallium-extra-hud=false

# Set environment to force regular archives
export ARFLAGS="--no-thin"
export AR="aarch64-none-elf-ar --no-thin"

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

# List built libraries
echo "📋 Built libraries:"
find . -name "*.a" -exec ls -lh {} \;

# Verify no shared libraries
echo "🔍 Checking for shared libraries (should find none):"
shared_libs=$(find . -name "*.so*" || true)
if [ -n "$shared_libs" ]; then
    echo "❌ WARNING: Found shared libraries:"
    echo "$shared_libs"
else
    echo "✅ No shared libraries found (good!)"
fi

echo ""
echo "🎉 Mesa build completed successfully!"
echo "📁 Libraries are in build-switch/src/"
echo "🔗 Ready for linking in your Switch project!" 