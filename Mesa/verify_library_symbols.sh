#!/bin/bash

echo "🔍 Comprehensive Mesa Library Symbol Verification"
echo "================================================"

# Check if we're in the right directory
if [ ! -d "Vita3K-Switch/external/custom-mesa/lib" ]; then
    echo "❌ Error: Vita3K-Switch/external/custom-mesa/lib not found"
    echo "Please run this script from the VitaNS root directory"
    exit 1
fi

LIB_DIR="Vita3K-Switch/external/custom-mesa/lib"

echo "📊 Checking library files..."
ls -la "$LIB_DIR"/*.a

echo ""
echo "🔍 Critical Symbol Verification:"
echo "================================"

# Check for _glapi_get_proc_address in libglapi_static.a
echo "1. Checking libglapi_static.a for _glapi_get_proc_address..."
if nm "$LIB_DIR/libglapi_static.a" 2>/dev/null | grep -q "_glapi_get_proc_address"; then
    echo "   ✅ _glapi_get_proc_address FOUND"
    nm "$LIB_DIR/libglapi_static.a" 2>/dev/null | grep "_glapi_get_proc_address"
else
    echo "   ❌ _glapi_get_proc_address MISSING"
    echo "   Available symbols in libglapi_static.a:"
    nm "$LIB_DIR/libglapi_static.a" 2>/dev/null | grep "_glapi" | head -5
fi

# Check for _eglDriver in libEGL.a
echo ""
echo "2. Checking libEGL.a for _eglDriver..."
if nm "$LIB_DIR/libEGL.a" 2>/dev/null | grep -q "_eglDriver"; then
    echo "   ✅ _eglDriver FOUND"
    nm "$LIB_DIR/libEGL.a" 2>/dev/null | grep "_eglDriver"
else
    echo "   ❌ _eglDriver MISSING"
    echo "   Available symbols in libEGL.a:"
    nm "$LIB_DIR/libEGL.a" 2>/dev/null | grep "_egl" | head -5
fi

# Check for threading symbols in libmesa_util.a
echo ""
echo "3. Checking libmesa_util.a for threading symbols..."
if nm "$LIB_DIR/libmesa_util.a" 2>/dev/null | grep -q "_simple_mtx_plain_init_once"; then
    echo "   ✅ _simple_mtx_plain_init_once FOUND"
else
    echo "   ❌ _simple_mtx_plain_init_once MISSING"
fi

if nm "$LIB_DIR/libmesa_util.a" 2>/dev/null | grep -q "util_call_once_data_slow"; then
    echo "   ✅ util_call_once_data_slow FOUND"
else
    echo "   ❌ util_call_once_data_slow MISSING"
fi

# Check for rwlock symbols
echo ""
echo "4. Checking for rwlock symbols..."
rwlock_symbols=("u_rwlock_rdlock" "u_rwlock_wrlock" "u_rwlock_rdunlock" "u_rwlock_wrunlock")
for symbol in "${rwlock_symbols[@]}"; do
    if nm "$LIB_DIR/libmesa_util.a" 2>/dev/null | grep -q "$symbol"; then
        echo "   ✅ $symbol FOUND"
    else
        echo "   ❌ $symbol MISSING"
    fi
done

# Check archive types
echo ""
echo "5. Checking archive types..."
for lib in "$LIB_DIR"/*.a; do
    if [ -f "$lib" ]; then
        echo "   $(basename "$lib"): $(file "$lib" | cut -d: -f2)"
    fi
done

# Check for undefined symbols
echo ""
echo "6. Checking for undefined symbols..."
echo "   Undefined symbols in libEGL.a:"
nm -u "$LIB_DIR/libEGL.a" 2>/dev/null | grep -E "(glapi|egl|simple_mtx|util_call)" | head -5

echo ""
echo "   Undefined symbols in libGLESv2.a:"
nm -u "$LIB_DIR/libGLESv2.a" 2>/dev/null | grep -E "(glapi|egl|simple_mtx|util_call)" | head -5

echo ""
echo "📋 Summary:"
echo "==========="
echo "If any ❌ items are found above, the Mesa libraries are incomplete."
echo "This explains why the CI build is failing with undefined reference errors."
echo ""
echo "🔧 Next Steps:"
echo "=============="
echo "1. If symbols are missing, rebuild Mesa with correct configuration"
echo "2. Ensure all dependencies are properly included in the build"
echo "3. Verify that libraries are regular archives (not thin)"
echo "4. Check that the build includes all required source files"
