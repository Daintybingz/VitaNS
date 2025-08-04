#!/bin/bash

echo "🔍 Mesa Completion Verification Script"
echo "====================================="
echo "Checking for issues mentioned in CUSTOM_MESA_COMPLETION_PLAN.md"
echo ""

# Check if build directory exists
if [ ! -d "build-switch" ]; then
    echo "❌ Error: build-switch directory not found. Run build_real_mesa.sh first."
    exit 1
fi

echo "📊 Library Status Check:"
echo "========================"

# Check all required libraries
libraries=(
    "build-switch/src/mapi/es2api/libGLESv2.a:OpenGL ES 2.0 API"
    "build-switch/src/mapi/glapi/libglapi_static.a:GL API dispatch"
    "build-switch/src/egl/libEGL.a:EGL interface"
    "build-switch/src/util/libmesa_util.a:Utilities, threading"
    "build-switch/src/gallium/drivers/softpipe/libsoftpipe.a:Software renderer"
    "build-switch/src/util/blake3/libblake3.a:Hashing"
    "build-switch/src/mesa/libmesa.a:Core Mesa"
)

for lib_info in "${libraries[@]}"; do
    IFS=':' read -r lib_path lib_name <<< "$lib_info"
    if [ -f "$lib_path" ]; then
        size=$(ls -lh "$lib_path" | awk '{print $5}')
        echo "✅ $lib_name: $size"
    else
        echo "❌ $lib_name: NOT FOUND"
    fi
done

echo ""
echo "🔍 Critical Symbol Check:"
echo "========================"

# Check for missing GL API dispatch system
echo "1. GL API Dispatch System:"
if [ -f "build-switch/src/mapi/glapi/libglapi_static.a" ]; then
    dispatch_found=$(nm build-switch/src/mapi/glapi/libglapi_static.a | grep -c "_glapi_tls_Dispatch" || echo "0")
    if [ "$dispatch_found" -gt 0 ]; then
        echo "   ✅ _glapi_tls_Dispatch found"
    else
        echo "   ❌ _glapi_tls_Dispatch MISSING"
    fi
    
    proc_addr_found=$(nm build-switch/src/mapi/glapi/libglapi_static.a | grep -c "_glapi_get_proc_address" || echo "0")
    if [ "$proc_addr_found" -gt 0 ]; then
        echo "   ✅ _glapi_get_proc_address found"
    else
        echo "   ❌ _glapi_get_proc_address MISSING"
    fi
else
    echo "   ❌ libglapi_static.a not found"
fi

# Check for missing utility functions
echo ""
echo "2. Utility Functions:"
if [ -f "build-switch/src/util/libmesa_util.a" ]; then
    rwlock_found=$(nm build-switch/src/util/libmesa_util.a | grep -c "u_rwlock" || echo "0")
    if [ "$rwlock_found" -gt 0 ]; then
        echo "   ✅ u_rwlock functions found"
    else
        echo "   ❌ u_rwlock functions MISSING"
    fi
    
    mtx_found=$(nm build-switch/src/util/libmesa_util.a | grep -c "_simple_mtx" || echo "0")
    if [ "$mtx_found" -gt 0 ]; then
        echo "   ✅ _simple_mtx functions found"
    else
        echo "   ❌ _simple_mtx functions MISSING"
    fi
    
    call_once_found=$(nm build-switch/src/util/libmesa_util.a | grep -c "util_call_once" || echo "0")
    if [ "$call_once_found" -gt 0 ]; then
        echo "   ✅ util_call_once functions found"
    else
        echo "   ❌ util_call_once functions MISSING"
    fi
else
    echo "   ❌ libmesa_util.a not found"
fi

# Check for missing system integration
echo ""
echo "3. System Integration:"
if [ -f "build-switch/src/util/libmesa_util.a" ]; then
    os_functions=("os_dupfd_cloexec" "debug_get_bool_option" "os_get_option")
    for func in "${os_functions[@]}"; do
        found=$(nm build-switch/src/util/libmesa_util.a | grep -c "$func" || echo "0")
        if [ "$found" -gt 0 ]; then
            echo "   ✅ $func found"
        else
            echo "   ❌ $func MISSING"
        fi
    done
else
    echo "   ❌ libmesa_util.a not found"
fi

# Check for missing EGL driver
echo ""
echo "4. EGL Driver:"
if [ -f "build-switch/src/egl/libEGL.a" ]; then
    egl_driver_found=$(nm build-switch/src/egl/libEGL.a | grep -c "_eglDriver" || echo "0")
    if [ "$egl_driver_found" -gt 0 ]; then
        echo "   ✅ _eglDriver found"
    else
        echo "   ❌ _eglDriver MISSING"
    fi
    
    egl_functions=("eglCreateContext" "eglMakeCurrent" "eglSwapBuffers")
    for func in "${egl_functions[@]}"; do
        found=$(nm build-switch/src/egl/libEGL.a | grep -c "$func" || echo "0")
        if [ "$found" -gt 0 ]; then
            echo "   ✅ $func found"
        else
            echo "   ❌ $func MISSING"
        fi
    done
else
    echo "   ❌ libEGL.a not found"
fi

echo ""
echo "📋 Summary:"
echo "==========="
echo "This verification checks for the specific issues mentioned in CUSTOM_MESA_COMPLETION_PLAN.md"
echo "If any ❌ items are found, the Mesa build needs to be updated with the correct configuration."
echo ""
echo "🔧 Next Steps:"
echo "=============="
echo "1. If missing symbols found, run: ./build_real_mesa.sh"
echo "2. If still missing, check meson configuration options"
echo "3. Verify that all libraries are regular archives (not thin)"
echo "4. Test linking with your Switch project" 