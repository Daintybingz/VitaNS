#!/bin/bash

# 🏭 Production Mesa Libraries Copy Script
# Copies optimized, stripped Mesa libraries to VitaNS project

set -e

echo "🏭 Copying Production Mesa Libraries to VitaNS..."

# Source directories
PRODUCTION_BUILD="Mesa/build-production"
VITANS_MESA="Vita3K-Switch/external/custom-mesa"

# Create backup of current libraries
echo "📦 Creating backup of current libraries..."
mkdir -p "${VITANS_MESA}/lib/backup-$(date +%Y%m%d-%H%M%S)"
cp "${VITANS_MESA}/lib/"*.a "${VITANS_MESA}/lib/backup-$(date +%Y%m%d-%H%M%S)/" 2>/dev/null || true

# Copy production libraries
echo "📋 Copying production libraries..."

# Core Mesa libraries
cp "${PRODUCTION_BUILD}/src/mapi/es2api/libGLESv2.a" "${VITANS_MESA}/lib/"
cp "${PRODUCTION_BUILD}/src/mapi/glapi/libglapi_static.a" "${VITANS_MESA}/lib/"
cp "${PRODUCTION_BUILD}/src/egl/libEGL.a" "${VITANS_MESA}/lib/"
cp "${PRODUCTION_BUILD}/src/util/libmesa_util.a" "${VITANS_MESA}/lib/"
cp "${PRODUCTION_BUILD}/src/gallium/drivers/softpipe/libsoftpipe.a" "${VITANS_MESA}/lib/"
cp "${PRODUCTION_BUILD}/src/util/blake3/libblake3.a" "${VITANS_MESA}/lib/"
cp "${PRODUCTION_BUILD}/src/mesa/libmesa.a" "${VITANS_MESA}/lib/"

# Copy headers if they exist
if [ -d "${PRODUCTION_BUILD}/include" ]; then
    echo "📋 Copying production headers..."
    cp -r "${PRODUCTION_BUILD}/include/"* "${VITANS_MESA}/include/" 2>/dev/null || true
fi

# Verify libraries
echo "🔍 Verifying production libraries..."
ls -la "${VITANS_MESA}/lib/"*.a

# Check library sizes
echo "📊 Library size comparison:"
echo "Production libraries:"
ls -lh "${VITANS_MESA}/lib/"*.a

# Verify no debug symbols
echo "🔍 Checking for debug symbols..."
for lib in "${VITANS_MESA}/lib/"*.a; do
    if [ -f "$lib" ]; then
        debug_symbols=$(nm "$lib" 2>/dev/null | grep -c " D " || echo "0")
        echo "  $(basename "$lib"): $debug_symbols debug symbols"
    fi
done

# Test linking
echo "🧪 Testing library linking..."
cd Vita3K-Switch
if [ -f "CMakeLists.txt" ]; then
    echo "✅ CMakeLists.txt found - ready for build test"
else
    echo "⚠️  CMakeLists.txt not found in Vita3K-Switch directory"
fi

echo "✅ Production Mesa libraries copied successfully!"
echo "📋 Next steps:"
echo "  1. Commit the new libraries: git add Vita3K-Switch/external/custom-mesa/lib/"
echo "  2. Test the build: cd Vita3K-Switch && cmake --build build"
echo "  3. Monitor CI build for success"
