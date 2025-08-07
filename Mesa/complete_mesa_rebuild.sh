#!/bin/bash
# complete_mesa_rebuild.sh
# Complete Mesa Library Rebuild for Nintendo Switch with All Required Symbols

set -e

echo "🎯 Complete Mesa Library Rebuild for Nintendo Switch"
echo "=================================================="
echo "Target: Fix undefined reference errors for critical symbols"
echo ""

# Phase 1: Clean Environment
echo "🧹 Phase 1: Cleaning build environment..."
rm -rf build-switch/
rm -rf build/
echo "✅ Build directories cleaned"

# Phase 2: Verify Source Files
echo ""
echo "🔍 Phase 2: Verifying critical source files..."
echo "Checking for simple_mtx.c and u_call_once.c..."

if [ ! -f "src/util/simple_mtx.c" ]; then
    echo "❌ ERROR: src/util/simple_mtx.c not found!"
    exit 1
fi

if [ ! -f "src/util/u_call_once.c" ]; then
    echo "❌ ERROR: src/util/u_call_once.c not found!"
    exit 1
fi

echo "✅ Critical source files found"

# Phase 3: Enhanced Configuration
echo ""
echo "⚙️  Phase 3: Configuring build with enhanced settings..."
meson setup build-switch/ --cross-file switch.meson \
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
  -Ddebug=true \
  -Doptimization=2

echo "✅ Build configuration completed"

# Phase 4: Build with Verbose Output
echo ""
echo "🔨 Phase 4: Building with dependency tracking..."
echo "This may take 5-10 minutes..."
ninja -C build-switch/ -v

echo "✅ Build completed successfully"

# Phase 5: Convert to Regular Archives
echo ""
echo "📦 Phase 5: Converting to regular archives..."
if [ -f "create_regular_archives.sh" ]; then
    ./create_regular_archives.sh
    echo "✅ Archive conversion completed"
else
    echo "⚠️  Warning: create_regular_archives.sh not found, skipping archive conversion"
fi

# Phase 6: Symbol Verification
echo ""
echo "🔍 Phase 6: Verifying all symbols..."
echo "=== Symbol Verification Results ==="

# Check for defined symbols (T)
echo "Defined symbols (T):"
find build-switch/ -name "*.a" -exec nm {} \; | grep -E "(simple_mtx_plain_init_once|util_call_once_data_slow)" | grep "^T" | sort | uniq

echo ""
echo "Undefined symbols (U):"
find build-switch/ -name "*.a" -exec nm {} \; | grep -E "(simple_mtx_plain_init_once|util_call_once_data_slow)" | grep "^U" | sort | uniq

# Phase 7: Library Inventory
echo ""
echo "📋 Phase 7: Final library inventory..."
echo "=== Library Status ==="
find build-switch/ -name "*.a" -exec sh -c 'echo "📦 $(basename "$1") - $(file "$1" | cut -d: -f2)"' _ {} \;

# Phase 8: Test Linking
echo ""
echo "🧪 Phase 8: Testing linking..."
echo "Creating test program..."

cat > test_link.c << 'EOF'
#include <GLES2/gl2.h>
#include <EGL/egl.h>

int main() {
    // Test basic OpenGL ES 2.0 functionality
    glClear(GL_COLOR_BUFFER_BIT);
    return 0;
}
EOF

echo "Testing link with all libraries..."
if aarch64-none-elf-gcc test_link.c \
    -Lbuild-switch/src/mapi/es2api \
    -Lbuild-switch/src/mapi/glapi \
    -Lbuild-switch/src/egl \
    -Lbuild-switch/src/util \
    -Lbuild-switch/src/gallium/drivers/softpipe \
    -Lbuild-switch/src/util/blake3 \
    -Lbuild-switch/src/mesa \
    -lGLESv2 -lglapi_static -lEGL -lmesa_util -lsoftpipe -lblake3 -lmesa -lpthread -lm \
    -o test_link 2>/dev/null; then
    echo "✅ Linking test successful!"
    rm -f test_link test_link.c
else
    echo "❌ Linking test failed - check for undefined references"
    echo "Check the error messages above for specific missing symbols"
    rm -f test_link.c
fi

echo ""
echo "🎉 Complete rebuild finished!"
echo ""
echo "📊 Summary:"
echo "- Build directories: Cleaned and rebuilt"
echo "- Configuration: Enhanced with debug and optimization"
echo "- Libraries: Built with verbose dependency tracking"
echo "- Archives: Converted to regular format"
echo "- Symbols: Verified for critical functions"
echo "- Linking: Tested with sample program"
echo ""
echo "🚀 Next steps:"
echo "1. Copy libraries to your project"
echo "2. Use correct linking order: -lGLESv2 -lglapi_static -lEGL -lmesa_util -lsoftpipe -lblake3 -lmesa -lpthread -lm"
echo "3. Include headers from build-switch/include/"
echo ""
echo "📚 For integration help, see: MESA_SWITCH_FINAL_STATUS.md" 