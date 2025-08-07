# 🎯 **Complete Library Rebuild Approach**

## 🚨 **Current Problem Analysis**

- **Issue**: Custom Mesa libraries missing critical symbols (`_simple_mtx_plain_init_once`, `util_call_once_data_slow`, etc.)
- **Root Cause**: Libraries built for different architecture or incomplete build process
- **Impact**: Build fails with undefined reference errors during linking
- **Target**: Nintendo Switch development with devkitPro toolchain

## 🔍 **Symbol Analysis Results**

### **✅ Symbols Found in libmesa_util.a:**
```
T _simple_mtx_plain_init_once    # Defined (text symbol)
T util_call_once_data_slow       # Defined (text symbol)
```

### **❌ Undefined References in Other Libraries:**
```
U _simple_mtx_plain_init_once    # Undefined in libEGL.a, libGLESv2.a, etc.
U util_call_once_data_slow       # Undefined in libEGL.a, libGLESv2.a, etc.
```

## 🛠️ **Complete Rebuild Strategy**

### **Phase 1: Clean Build Environment**
```bash
# Remove all previous build artifacts
rm -rf build-switch/
rm -rf build/

# Clean any cached build files
meson setup --wipe build-switch/ --cross-file switch.meson
```

### **Phase 2: Enhanced Build Configuration**
```bash
# Build with complete dependency tracking
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
```

### **Phase 3: Dependency Verification**
```bash
# Verify all required source files are included
find src/util/ -name "*.c" | grep -E "(simple_mtx|u_call_once)"
find src/util/ -name "*.h" | grep -E "(simple_mtx|u_call_once)"
```

### **Phase 4: Build with Dependency Tracking**
```bash
# Build with verbose output to track dependencies
ninja -C build-switch/ -v
```

### **Phase 5: Symbol Verification**
```bash
# Verify all symbols are properly defined
find build-switch/ -name "*.a" -exec nm {} \; | grep -E "(simple_mtx_plain_init_once|util_call_once_data_slow)" | sort | uniq
```

## 🔧 **Enhanced Build Script**

```bash
#!/bin/bash
# complete_mesa_rebuild.sh

set -e

echo "🎯 Complete Mesa Library Rebuild for Nintendo Switch"
echo "=================================================="

# Phase 1: Clean Environment
echo "🧹 Cleaning build environment..."
rm -rf build-switch/
rm -rf build/

# Phase 2: Enhanced Configuration
echo "⚙️  Configuring build with enhanced settings..."
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

# Phase 3: Build with Verbose Output
echo "🔨 Building with dependency tracking..."
ninja -C build-switch/ -v

# Phase 4: Convert to Regular Archives
echo "📦 Converting to regular archives..."
./create_regular_archives.sh

# Phase 5: Symbol Verification
echo "🔍 Verifying all symbols..."
echo "=== Symbol Verification Results ==="
find build-switch/ -name "*.a" -exec nm {} \; | grep -E "(simple_mtx_plain_init_once|util_call_once_data_slow)" | sort | uniq

echo "✅ Complete rebuild finished!"
```

## 📋 **Critical Files to Verify**

### **1. src/util/simple_mtx.c**
```c
// Must contain _simple_mtx_plain_init_once implementation
void
_simple_mtx_plain_init_once(struct simple_mtx *mtx, bool recursive)
{
   // Implementation should be present
}
```

### **2. src/util/u_call_once.c**
```c
// Must contain util_call_once_data_slow implementation
void
util_call_once_data_slow(once_flag *flag, void (*func)(void *), void *data)
{
   // Implementation should be present
}
```

### **3. src/util/meson.build**
```meson
# Must include these files in files_mesa_util
files_mesa_util = files(
  # ... other files ...
  'simple_mtx.c',
  'simple_mtx.h',
  'u_call_once.c',
  'u_call_once.h',
  # ... other files ...
)
```

## 🔍 **Linking Order Fix**

### **Correct Library Linking Order:**
```bash
# Order matters! libmesa_util.a must come before libraries that depend on it
-lGLESv2 -lglapi_static -lEGL -lmesa_util -lsoftpipe -lblake3 -lmesa -lpthread -lm
```

### **CMakeLists.txt Example:**
```cmake
target_link_libraries(YourProject PRIVATE
    # Mesa libraries in dependency order
    ${PROJECT_SOURCE_DIR}/lib/libGLESv2.a
    ${PROJECT_SOURCE_DIR}/lib/libglapi_static.a
    ${PROJECT_SOURCE_DIR}/lib/libEGL.a
    ${PROJECT_SOURCE_DIR}/lib/libmesa_util.a      # Must come before EGL/GLES2
    ${PROJECT_SOURCE_DIR}/lib/libsoftpipe.a
    ${PROJECT_SOURCE_DIR}/lib/libblake3.a
    ${PROJECT_SOURCE_DIR}/lib/libmesa.a
    # System dependencies
    -lpthread -lm
)
```

## 🚀 **Execution Plan**

### **Step 1: Run Complete Rebuild**
```bash
chmod +x complete_mesa_rebuild.sh
./complete_mesa_rebuild.sh
```

### **Step 2: Verify Symbols**
```bash
# Check that all symbols are defined (T) not undefined (U)
find build-switch/ -name "*.a" -exec nm {} \; | grep -E "(simple_mtx_plain_init_once|util_call_once_data_slow)" | grep "^T"
```

### **Step 3: Test Linking**
```bash
# Test linking with a simple program
echo '#include <GLES2/gl2.h>
int main() { return 0; }' > test_link.c
aarch64-none-elf-gcc test_link.c -Lbuild-switch/src/mapi/es2api -Lbuild-switch/src/mapi/glapi -Lbuild-switch/src/egl -Lbuild-switch/src/util -Lbuild-switch/src/gallium/drivers/softpipe -Lbuild-switch/src/util/blake3 -Lbuild-switch/src/mesa -lGLESv2 -lglapi_static -lEGL -lmesa_util -lsoftpipe -lblake3 -lmesa -lpthread -lm -o test_link
```

## 🎯 **Expected Results**

After complete rebuild:
- ✅ All symbols defined (T) not undefined (U)
- ✅ No undefined reference errors during linking
- ✅ All libraries are regular archives (not thin)
- ✅ Complete OpenGL ES 2.0 and EGL functionality
- ✅ Switch compatibility verified

## 🔧 **Troubleshooting**

### **If symbols are still undefined:**
1. Check that source files are actually compiled
2. Verify linking order is correct
3. Ensure all dependencies are built
4. Check for build configuration issues

### **If build fails:**
1. Check devkitPro installation
2. Verify cross-compilation toolchain
3. Check Mesa source code integrity
4. Review build logs for specific errors

---

**Status**: 🟡 **READY FOR EXECUTION**  
**Next Step**: Run complete rebuild script  
**Expected Outcome**: All symbols properly defined and linked 