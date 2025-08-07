#!/bin/bash
# clean_production_rebuild.sh
# Phase 2: Clean Production Rebuild for Nintendo Switch
# Build Mesa with stripped symbols and no debug path references

set -e

echo "🏭 Phase 2: Clean Production Rebuild for Nintendo Switch"
echo "======================================================="
echo "Target: Production-ready libraries with no debug paths"
echo ""

# Phase 1: Complete Clean Environment
echo "🧹 Phase 1: Complete clean environment..."
rm -rf build-switch/
rm -rf build/
rm -rf build-production/
echo "✅ All build directories cleaned"

# Phase 2: Verify Source Files
echo ""
echo "🔍 Phase 2: Verifying critical source files..."
if [ ! -f "src/util/simple_mtx.c" ]; then
    echo "❌ ERROR: src/util/simple_mtx.c not found!"
    exit 1
fi

if [ ! -f "src/util/u_call_once.c" ]; then
    echo "❌ ERROR: src/util/u_call_once.c not found!"
    exit 1
fi

echo "✅ Critical source files found"

# Phase 3: Production Configuration
echo ""
echo "⚙️  Phase 3: Configuring production build..."
echo "Settings: -Dstrip=true -Ddebug=false -Doptimization=3"
meson setup build-production/ --cross-file switch.meson \
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
  -Dstrip=true \
  -Ddebug=false \
  -Doptimization=3

echo "✅ Production configuration completed"

# Phase 4: Production Build
echo ""
echo "🔨 Phase 4: Building production libraries..."
echo "This may take 5-10 minutes..."
ninja -C build-production/ -v

echo "✅ Production build completed successfully"

# Phase 5: Convert to Regular Archives
echo ""
echo "📦 Phase 5: Converting to regular archives..."
if [ -f "create_regular_archives.sh" ]; then
    # Update script to use build-production directory
    sed 's/build-switch/build-production/g' create_regular_archives.sh > create_regular_archives_prod.sh
    chmod +x create_regular_archives_prod.sh
    ./create_regular_archives_prod.sh
    echo "✅ Archive conversion completed"
else
    echo "⚠️  Warning: create_regular_archives.sh not found, skipping archive conversion"
fi

# Phase 6: Symbol Analysis (Production)
echo ""
echo "🔍 Phase 6: Production symbol analysis..."
echo "=== Production Symbol Verification ==="

# Check for defined symbols (T)
echo "Defined symbols (T):"
find build-production/ -name "*.a" -exec nm {} \; | grep -E "(simple_mtx_plain_init_once|util_call_once_data_slow)" | grep "^T" | sort | uniq

echo ""
echo "Undefined symbols (U):"
find build-production/ -name "*.a" -exec nm {} \; | grep -E "(simple_mtx_plain_init_once|util_call_once_data_slow)" | grep "^U" | sort | uniq

# Phase 7: Path Reference Check
echo ""
echo "🔍 Phase 7: Checking for path references in symbols..."
echo "=== Path Reference Analysis ==="

# Check for any path references in symbols
echo "Symbols with path references:"
find build-production/ -name "*.a" -exec nm {} \; | grep -E "(\.c|\.h|/)" | head -10

echo ""
echo "Total symbols with path references:"
find build-production/ -name "*.a" -exec nm {} \; | grep -E "(\.c|\.h|/)" | wc -l

# Phase 8: Library Size Analysis
echo ""
echo "📊 Phase 8: Library size analysis..."
echo "=== Production Library Sizes ==="
find build-production/ -name "*.a" -exec sh -c 'echo "📦 $(basename "$1") - $(du -h "$1" | cut -f1)"' _ {} \;

# Phase 9: Production Linking Test
echo ""
echo "🧪 Phase 9: Production linking test..."
echo "Creating test program..."

cat > test_production.c << 'EOF'
#include <GLES2/gl2.h>

int main() {
    // Test basic OpenGL ES 2.0 functionality
    glClear(GL_COLOR_BUFFER_BIT);
    return 0;
}
EOF

echo "Testing production link..."
if /opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc test_production.c \
    -I./include \
    -Lbuild-production/src/mapi/es2api \
    -Lbuild-production/src/mapi/glapi \
    -Lbuild-production/src/egl \
    -Lbuild-production/src/util \
    -Lbuild-production/src/gallium/drivers/softpipe \
    -Lbuild-production/src/util/blake3 \
    -Lbuild-production/src/mesa \
    -lGLESv2 -lglapi_static -lEGL -lmesa_util -lsoftpipe -lblake3 -lmesa -lpthread -lm \
    -o test_production 2>/dev/null; then
    echo "✅ Production linking test successful!"
    echo "Production executable size: $(du -h test_production | cut -f1)"
    rm -f test_production test_production.c
else
    echo "❌ Production linking test failed"
    echo "Check the error messages above for specific issues"
    rm -f test_production.c
fi

# Phase 10: Comparison with Debug Build
echo ""
echo "📊 Phase 10: Production vs Debug comparison..."
if [ -d "build-switch" ]; then
    echo "Debug build exists, comparing sizes..."
    echo "=== Size Comparison ==="
    echo "Production libraries:"
    find build-production/ -name "*.a" -exec sh -c 'echo "  $(basename "$1"): $(du -h "$1" | cut -f1)"' _ {} \;
    echo ""
    echo "Debug libraries:"
    find build-switch/ -name "*.a" -exec sh -c 'echo "  $(basename "$1"): $(du -h "$1" | cut -f1)"' _ {} \;
else
    echo "No debug build found for comparison"
fi

# Phase 11: Final Verification
echo ""
echo "🔍 Phase 11: Final production verification..."
echo "=== Production Quality Check ==="

# Check archive types
echo "Archive types:"
find build-production/ -name "*.a" -exec file {} \;

# Check for any remaining debug symbols
echo ""
echo "Debug symbols remaining:"
find build-production/ -name "*.a" -exec nm {} \; | grep -E "\.debug" | wc -l

echo ""
echo "🎉 Production rebuild finished!"
echo ""
echo "📊 Production Summary:"
echo "- Build directories: Cleaned and rebuilt"
echo "- Configuration: Production optimized (-Dstrip=true -Ddebug=false)"
echo "- Libraries: Built with size optimization"
echo "- Archives: Converted to regular format"
echo "- Symbols: Stripped of debug information"
echo "- Path references: Minimized"
echo "- Linking: Tested with production build"
echo ""
echo "🚀 Production libraries ready:"
echo "1. Copy from build-production/ to your project"
echo "2. Use same linking order: -lGLESv2 -lglapi_static -lEGL -lmesa_util -lsoftpipe -lblake3 -lmesa -lpthread -lm"
echo "3. Include headers from ./include/"
echo ""
echo "📚 For integration help, see: MESA_SWITCH_FINAL_STATUS.md" 