#!/bin/bash

# Simple Mesa Build for Nintendo Switch
# Focuses on building target libraries with regular static archives

set -e

echo "🎮 Simple Mesa Build for Nintendo Switch"
echo "📦 Building with regular static archives (not thin archives)"

# Set toolchain paths
CC="/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc"
AR="/opt/devkitpro/devkitA64/bin/aarch64-none-elf-ar"

# Check if toolchain exists
if [ ! -f "$CC" ]; then
    echo "❌ Error: devkitPro toolchain not found at $CC"
    echo "   Please install devkitPro and devkitA64"
    exit 1
fi

# Clean previous build
if [ -d "build-switch" ]; then
    echo "🧹 Cleaning previous build..."
    rm -rf build-switch
fi

# Create build directory
mkdir -p build-switch

echo "🔨 Building target libraries..."

# Build libmesa_util.a
echo "Building libmesa_util.a..."
cd build-switch
mkdir -p src/util
cd src/util

# Compile util sources (only files that exist)
$CC -c \
  -I/opt/devkitpro/libnx/include \
  -I../../../include \
  -I../../../src \
  -D__SWITCH__ \
  -O2 \
  -ffunction-sections \
  -fdata-sections \
  ../../../src/util/os_time.c \
  ../../../src/util/os_misc.c \
  ../../../src/util/rwlock.c \
  ../../../src/util/anon_file.c \
  ../../../src/util/u_dl.c

# Create regular static archive (devkitPro ar doesn't create thin archives by default)
$AR rcs libmesa_util.a *.o

echo "✅ Built libmesa_util.a"

# Build libglapi_static.a
echo "Building libglapi_static.a..."
cd ..
mkdir -p mapi/glapi
cd mapi/glapi

# Compile GLAPI sources
$CC -c \
  -I/opt/devkitpro/libnx/include \
  -I../../../../include \
  -I../../../../src \
  -D__SWITCH__ \
  -O2 \
  -ffunction-sections \
  -fdata-sections \
  ../../../../src/mapi/glapi/glapi.c \
  ../../../../src/mapi/glapi/glapi_dispatch.c \
  ../../../../src/mapi/glapi/glapi_entrypoint.c \
  ../../../../src/mapi/glapi/glapi_getproc.c \
  ../../../../src/mapi/glapi/glapi_nop.c \
  ../../../../src/mapi/u_current.c

# Create regular static archive
$AR rcs libglapi_static.a *.o

echo "✅ Built libglapi_static.a"

# Build libGLESv2.a
echo "Building libGLESv2.a..."
cd ..
mkdir -p es2api
cd es2api

# Compile GLES2 sources
$CC -c \
  -I/opt/devkitpro/libnx/include \
  -I../../../../include \
  -I../../../../src \
  -D__SWITCH__ \
  -O2 \
  -ffunction-sections \
  -fdata-sections \
  ../../../../src/mapi/entry.c

# Create regular static archive
$AR rcs libGLESv2.a *.o

echo "✅ Built libGLESv2.a"

# Build libmesa.a (stub)
echo "Building libmesa.a (stub)..."
cd ../../..
mkdir -p mesa
cd mesa

# Create stub source
cat > stub_mesa.c << 'EOF'
/*
 * Minimal Mesa stub for Nintendo Switch
 * This provides basic symbols for linking without the full Mesa implementation
 */

#include <stddef.h>

/* Basic stub functions to satisfy linking */
void mesa_stub_init(void) {
    /* Empty stub */
}

void mesa_stub_cleanup(void) {
    /* Empty stub */
}

/* Export some basic symbols that might be needed */
int mesa_stub_version = 1;
EOF

# Compile stub
$CC -c \
  -I/opt/devkitpro/libnx/include \
  -I../../../include \
  -I../../../src \
  -D__SWITCH__ \
  -O2 \
  -ffunction-sections \
  -fdata-sections \
  stub_mesa.c

# Create regular static archive
$AR rcs libmesa.a *.o

echo "✅ Built libmesa.a (stub)"

# Build libEGL.a
echo "Building libEGL.a..."
cd ..
mkdir -p egl
cd egl

# Compile EGL sources
$CC -c \
  -I/opt/devkitpro/libnx/include \
  -I../../../include \
  -I../../../src \
  -D__SWITCH__ \
  -O2 \
  -ffunction-sections \
  -fdata-sections \
  ../../../src/egl/main/eglcurrent.c \
  ../../../src/egl/main/eglapi.c \
  ../../../src/egl/main/eglconfig.c \
  ../../../src/egl/main/eglcontext.c \
  ../../../src/egl/main/egldisplay.c \
  ../../../src/egl/main/egldriver.c \
  ../../../src/egl/main/eglfallbacks.c \
  ../../../src/egl/main/eglglobals.c \
  ../../../src/egl/main/eglimage.c \
  ../../../src/egl/main/egllog.c \
  ../../../src/egl/main/eglmisc.c \
  ../../../src/egl/main/eglmode.c \
  ../../../src/egl/main/eglscreen.c \
  ../../../src/egl/main/eglsurface.c \
  ../../../src/egl/main/eglsync.c

# Create regular static archive
$AR rcs libEGL.a *.o

echo "✅ Built libEGL.a"

cd ../..

echo ""
echo "🎉 Simple build completed!"
echo "📋 Built libraries:"

# List all built libraries
find . -name "*.a" -exec ls -lh {} \;

echo ""
echo "🔍 Verifying archive types..."

# Check for thin archives
thin_archives=$(find . -name "*.a" -exec file {} \; | grep "thin archive" || true)
if [ -n "$thin_archives" ]; then
    echo "❌ WARNING: Found thin archives:"
    echo "$thin_archives"
else
    echo "✅ All archives are regular static archives (good!)"
fi

echo ""
echo "📁 Libraries are in build-switch/src/"
echo "🔗 Ready for linking in your Switch project!" 