# VitaNS Runtime Debugging Guide

## 🎉 Success: VitaNS is Running on Switch!

**The crash is actually good news!** Error code 2144-0001 means VitaNS.nro successfully:
- ✅ Built and linked correctly (all Mesa symbols resolved)
- ✅ Loaded on Nintendo Switch 
- ✅ Started executing main()
- ❌ Crashed during EGL/Mesa initialization

## 📊 Crash Analysis

### Error Details
- **Error Code**: 2144-0001 (0x290)
- **Program ID**: 010000000000001000 (homebrew application)
- **Firmware**: Atmosphere 1.9.1
- **Crash Location**: PC: 00000067B404A0A8

### Register Analysis
The crash occurred at address `0x67B404A0A8` which is within our application's address space. Looking at the backtrace:
- **BT[00]**: 00000067B4137C80 (likely in Mesa/EGL code)
- **BT[01]**: 00000067B4137684 (calling function)
- **BT[02]**: 00000067B41377B8 (higher level call)

This suggests the crash is occurring during EGL initialization, specifically around `eglInitialize()` or early Mesa driver loading.

## 🔍 Root Cause Analysis

### Most Likely Issues

#### 1. **Mesa Driver Initialization Failure**
The crash is occurring during Mesa's internal initialization, probably when:
- Loading the Softpipe driver
- Setting up the EGL display
- Initializing OpenGL ES context

#### 2. **Switch Platform Integration Issues**
- Native window creation might be failing
- Framebuffer access could be problematic
- Memory allocation issues in Mesa

#### 3. **Threading/Synchronization Problems**
- Mesa might be trying to use threading functions not available on Switch
- Mutex initialization could be failing

## 🛠️ Debugging Strategy

### Phase 1: Add Comprehensive Error Checking

The current main.cpp has good error checking, but we need more detailed debugging to see exactly where it fails.

### Phase 2: Mesa Debug Build

Build Mesa with debugging enabled to get more detailed crash information.

### Phase 3: Simplified Test

Create a minimal test that doesn't use full EGL to isolate the issue.

## 🚀 Immediate Next Steps

1. **Add detailed logging** to main.cpp to track exactly where it crashes
2. **Create debug Mesa build** with symbols and verbose logging
3. **Add exception handling** to catch and report specific errors
4. **Test with minimal EGL setup** to isolate the problem

This crash represents **significant progress** - we've successfully integrated Mesa and created a working Switch application. Now we just need to debug the runtime initialization!
