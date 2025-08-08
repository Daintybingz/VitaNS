 # Nintendo Switch Mesa Library Rebuild TODO

## 🎯 **Project Goal**
Rebuild Mesa libraries specifically for Nintendo Switch (aarch64-none-elf) to resolve all undefined reference errors and ensure complete symbol compatibility.

## 📋 **Current Problem Analysis**
- **Issue**: Custom Mesa libraries missing critical symbols (`_simple_mtx_plain_init_once`, `util_call_once_data_slow`, etc.)
- **Root Cause**: Libraries built for different architecture or incomplete build process
- **Impact**: Build fails with undefined reference errors during linking
- **Target**: Nintendo Switch development with devkitPro toolchain

## 🚀 **Recommended Approach: Complete Library Rebuild**

### **Phase 1: Environment Setup** ✅
- [x] Verify devkitPro installation
- [x] Confirm Switch toolchain availability
- [x] Set up build environment variables
- [ ] Create isolated build directory
- [ ] Install Mesa build dependencies

### **Phase 2: Mesa Source Preparation** 🔄
- [ ] Download Mesa source code (latest stable version)
- [ ] Apply Switch-specific patches if needed
- [ ] Configure for aarch64-none-elf target
- [ ] Set up build configuration for static libraries
- [ ] Verify source tree integrity

### **Phase 3: Build Configuration** 📝
- [ ] Create custom build script for Switch
- [ ] Configure Mesa for software rendering only
- [ ] Enable only required components (EGL, GLESv2, util)
- [ ] Set up proper include paths
- [ ] Configure for static linking

### **Phase 4: Library Build Process** 🔨
- [ ] Build Mesa with Switch toolchain
- [ ] Generate all required static libraries:
  - [ ] libEGL.a
  - [ ] libGLESv2.a
  - [ ] libglapi_static.a
  - [ ] libmesa_util.a
  - [ ] libsoftpipe.a
  - [ ] libblake3.a
  - [ ] libmesa.a
- [ ] Verify all symbols are present
- [ ] Test library compatibility

### **Phase 5: Symbol Verification** 🔍
- [ ] Use `nm` to check for required symbols:
  - [ ] `_simple_mtx_plain_init_once`
  - [ ] `util_call_once_data_slow`
  - [ ] `u_rwlock_rdlock`
  - [ ] `u_rwlock_rdunlock`
  - [ ] `_glapi_get_proc_address`
  - [ ] `simple_mtx_init`
  - [ ] `u_rwlock_init`
  - [ ] `os_dupfd_cloexec`
- [ ] Verify no missing dependencies
- [ ] Check for duplicate symbols

### **Phase 6: Integration Testing** 🧪
- [ ] Replace existing custom Mesa libraries
- [ ] Test build with VitaNS project
- [ ] Verify no undefined reference errors
- [ ] Test basic functionality
- [ ] Validate Switch compatibility

### **Phase 7: CI/CD Integration** 🔄
- [ ] Update GitHub Actions workflow
- [ ] Remove complex Mesa blocking logic
- [ ] Simplify build process
- [ ] Add library verification steps
- [ ] Test automated build pipeline

## 📁 **File Structure**
```
Mesa/
├── Switch_Mesa_Rebuild_TODO.md          # This file
├── build_script.sh                      # Build script for Switch Mesa
├── patches/                             # Switch-specific patches
│   ├── switch-compatibility.patch
│   └── aarch64-optimizations.patch
├── config/                              # Build configurations
│   ├── switch-mesa-config.txt
│   └── mesa-options.txt
└── output/                              # Built libraries
    ├── lib/
    │   ├── libEGL.a
    │   ├── libGLESv2.a
    │   └── ...
    └── include/
        ├── EGL/
        ├── GLES2/
        └── ...
```

## 🔧 **Technical Requirements**

### **Build Environment**
- **OS**: Ubuntu 20.04+ or similar Linux
- **Toolchain**: devkitPro Switch toolchain
- **Compiler**: aarch64-none-elf-gcc
- **Mesa Version**: Latest stable (recommend 23.x+)
- **Build Type**: Static libraries only

### **Required Components**
- **EGL**: OpenGL ES interface
- **GLESv2**: OpenGL ES 2.0 implementation
- **Mesa Util**: Utility functions
- **Softpipe**: Software renderer
- **GLAPI**: OpenGL API layer

### **Symbol Requirements**
All symbols must be present and properly linked:
```bash
# Required symbols to verify
nm libEGL.a | grep -E "(simple_mtx|util_call_once|u_rwlock|glapi)"
nm libGLESv2.a | grep -E "(simple_mtx|util_call_once|u_rwlock|glapi)"
```

## 📝 **Build Script Template**
```bash
#!/bin/bash
# Switch Mesa Build Script

set -e

# Configuration
MESA_VERSION="23.3.0"
BUILD_DIR="$(pwd)/mesa-build"
INSTALL_DIR="$(pwd)/custom-mesa"
TOOLCHAIN_PREFIX="aarch64-none-elf"

# Environment setup
export DEVKITPRO="/opt/devkitpro"
export DEVKITA64="$DEVKITPRO/devkitA64"
export PATH="$DEVKITA64/bin:$PATH"

# Download and extract Mesa
wget "https://mesa.freedesktop.org/archive/mesa-${MESA_VERSION}.tar.xz"
tar -xf "mesa-${MESA_VERSION}.tar.xz"
cd "mesa-${MESA_VERSION}"

# Configure for Switch
meson setup \
    --prefix="$INSTALL_DIR" \
    --cross-file="$DEVKITPRO/cmake/Switch.cmake" \
    -Dplatforms= \
    -Dgallium-drivers=swrast \
    -Dvulkan-drivers= \
    -Ddri-drivers= \
    -Dglx=disabled \
    -Dosmesa=false \
    -Dtools= \
    -Dgallium-va=disabled \
    -Dgallium-vdpau=disabled \
    -Dgallium-xa=disabled \
    -Dgallium-nine=false \
    -Dgallium-opencl=disabled \
    -Dgallium-rusticl=disabled \
    -Dgallium-shader-cache=disabled \
    -Dgallium-vm=disabled \
    -Dgles1=disabled \
    -Dgles2=enabled \
    -Dglx=disabled \
    -Dglvnd=false \
    -Dllvm=disabled \
    -Dshared-glapi=disabled \
    -Dlmsensors=disabled \
    -Dbuild-tests=false \
    -Dinstall-tests=false \
    "$BUILD_DIR"

# Build
ninja -C "$BUILD_DIR"

# Install
ninja -C "$BUILD_DIR" install

echo "Mesa build complete for Switch!"
```

## ⚠️ **Common Issues and Solutions**

### **Issue 1: Missing Symbols**
**Problem**: Libraries missing required symbols
**Solution**: Ensure all Mesa components are built and linked properly

### **Issue 2: Architecture Mismatch**
**Problem**: Libraries built for wrong architecture
**Solution**: Use correct toolchain and verify target

### **Issue 3: Dependency Conflicts**
**Problem**: System Mesa interfering with custom libraries
**Solution**: Complete isolation in build environment

### **Issue 4: Build Failures**
**Problem**: Mesa build process failing
**Solution**: Check dependencies, toolchain, and configuration

## 🎯 **Success Criteria**
- [ ] All required symbols present in libraries
- [ ] No undefined reference errors during linking
- [ ] VitaNS builds successfully for Switch
- [ ] Basic functionality works
- [ ] CI/CD pipeline passes consistently

## 📅 **Timeline Estimate**
- **Phase 1-2**: 1-2 days (setup and preparation)
- **Phase 3-4**: 2-3 days (build and configuration)
- **Phase 5-6**: 1-2 days (testing and verification)
- **Phase 7**: 1 day (CI/CD integration)
- **Total**: 5-8 days for complete solution

## 🔄 **Next Steps**
1. **Immediate**: Set up build environment
2. **Short-term**: Download and configure Mesa source
3. **Medium-term**: Build and test libraries
4. **Long-term**: Integrate with CI/CD pipeline

---

**Last Updated**: [Current Date]
**Status**: Planning Phase
**Priority**: High
**Assigned**: Development Team 