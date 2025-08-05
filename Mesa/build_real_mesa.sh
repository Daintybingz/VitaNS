#!/bin/bash
set -e

echo "🎯 Building REAL Mesa implementations for Switch..."
echo "This will create libraries with actual OpenGL ES 2.0 and EGL functions"

# Check if we're in the right directory
if [ ! -f "meson.build" ]; then
    echo "❌ Error: Must run from Mesa root directory"
    exit 1
fi

# Set environment variables to force regular archives (not thin archives)
echo "🔧 Setting up environment for regular archives..."
export ARFLAGS="--no-thin"
export AR="/opt/devkitpro/devkitA64/bin/aarch64-none-elf-ar --no-thin"

# Clean previous build
echo "🧹 Cleaning previous build..."
rm -rf build-switch

# Configure with real implementations - FIXED for static libraries
echo "⚙️  Configuring Mesa with real implementations..."
meson setup build-switch --cross-file switch.meson \
  -Dgles2=enabled \
  -Degl=enabled \
  -Dgallium-drivers=swrast \
  -Dglx=disabled \
  -Dgbm=disabled \
  -Dllvm=disabled \
  -Dvulkan-drivers=[] \
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
  -Dpower8=disabled \
  -Dvmware-mks-stats=false \
  -Dcpp-rtti=false \
  -Dallow-kcmp=disabled \
  -Dmesa-debug=false \
  -Dshader-cache-default=false \
  -Dsse2=false \
  -Ddraw-use-llvm=false \
  -Dshared-llvm=disabled \
  -Dopencl-spirv=false \
  -Dgallium-extra-hud=false \
  -Dshared-glapi=disabled

echo "🔨 Building with real implementations..."
ninja -C build-switch

echo "✅ Real Mesa build completed!"
echo ""
echo "📊 Checking library sizes and contents..."

# Check library sizes
echo "Library sizes:"
find build-switch -name "*.a" -exec ls -lh {} \;

# Check for real function implementations
echo ""
echo "🔍 Checking for real function implementations..."

# Check EGL functions
if [ -f "build-switch/src/egl/libEGL.a" ]; then
    echo "EGL functions found:"
    nm build-switch/src/egl/libEGL.a | grep -E "(eglCreateContext|eglMakeCurrent|eglSwapBuffers)" | head -5
fi

# Check OpenGL ES functions
if [ -f "build-switch/src/mapi/es2api/libGLESv2.a" ]; then
    echo "OpenGL ES functions found:"
    nm build-switch/src/mapi/es2api/libGLESv2.a | grep -E "(glClear|glDrawArrays|glTexImage2D)" | head -5
fi

# Check GLAPI functions - CRITICAL for dispatch
if [ -f "build-switch/src/mapi/glapi/libglapi_static.a" ]; then
    echo "GLAPI functions found:"
    nm build-switch/src/mapi/glapi/libglapi_static.a | grep -E "_glapi" | head -5
    echo ""
    echo "🔍 Checking for critical dispatch symbols:"
    nm build-switch/src/mapi/glapi/libglapi_static.a | grep -E "(_glapi_tls_Dispatch|_glapi_get_proc_address)" || echo "❌ Missing critical dispatch symbols!"
fi

# Check utility functions
if [ -f "build-switch/src/util/libmesa_util.a" ]; then
    echo ""
    echo "🔍 Checking utility functions:"
    nm build-switch/src/util/libmesa_util.a | grep -E "(u_rwlock|_simple_mtx)" | head -5
fi

# Check archive types
echo ""
echo "🔍 Checking archive types (should be regular, not thin):"
find build-switch -name "*.a" -exec file {} \; | head -5

echo ""
echo "🎉 Real Mesa build verification completed!"
echo "Libraries should now contain actual implementations instead of stubs." 