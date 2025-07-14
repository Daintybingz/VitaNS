# Nintendo Switch Testing Guide

## Overview

This document explains how testing works for the VitaNS project specifically on the Nintendo Switch platform, both in the GitHub Actions CI environment and for local development.

## Testing Architecture for Switch

### CI/CD Testing (GitHub Actions)

The VitaNS project uses a comprehensive testing strategy in the GitHub Actions CI environment:

#### **1. Build Environment**
- **Container**: `devkitpro/devkita64:latest` (official devkitPro Docker image)
- **Platform**: Ubuntu with Switch toolchain
- **Dependencies**: Automatically installed via `dkp-pacman`

#### **2. Test Execution Flow**
```
GitHub Actions Workflow
├── Environment Setup
│   ├── Checkout code
│   ├── Install Switch portlibs
│   └── Install testing dependencies (switch-gtest)
├── Build Phase
│   ├── Build VitaNS.nro (main executable)
│   └── Build VitaNSTests (test executable)
├── Test Execution
│   ├── Unit Tests (RendererTest*)
│   ├── Integration Tests (EmulatorIntegrationTest*)
│   ├── Performance Tests (PerformanceTest*)
│   └── Compatibility Tests (CompatibilityTest*)
└── Artifact Upload
    ├── Test results (XML format)
    ├── VitaNS.nro build
    └── Test executable
```

#### **3. Test Categories**

**Unit Tests** (`RendererTest.cpp`)
- **Purpose**: Test individual renderer components
- **Timeout**: 300 seconds (5 minutes)
- **Coverage**: Interface compliance, error handling, resource management
- **Example**: Testing renderer initialization, frame rendering, cleanup

**Integration Tests** (`EmulatorIntegrationTest.cpp`)
- **Purpose**: Test component interactions
- **Timeout**: 600 seconds (10 minutes)
- **Coverage**: Emulator-renderer integration, GPU subsystem
- **Example**: Full emulator initialization with renderer backend

**Performance Tests** (`PerformanceTest.cpp`)
- **Purpose**: Validate performance benchmarks
- **Timeout**: 1200 seconds (20 minutes)
- **Coverage**: Frame rates, initialization times, memory usage
- **Example**: GLES2 renderer achieving 30+ FPS target

**Compatibility Tests** (`compatibility_test.cpp`)
- **Purpose**: Test game compatibility
- **Timeout**: 900 seconds (15 minutes)
- **Coverage**: Game-specific rendering, shader compatibility
- **Example**: Testing specific game rendering scenarios

### **4. CI Workflow Commands**

```bash
# Build and test in GitHub Actions
name: Build and Test VitaNS

# Install dependencies
dkp-pacman -S --noconfirm switch-gtest switch-catch2

# Build main executable
cmake .. -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake
make -j$(nproc)

# Build and run tests
make VitaNSTests -j$(nproc)
./VitaNSTests --gtest_filter=RendererTest* --gtest_output=xml:test-results-renderer.xml
./VitaNSTests --gtest_filter=EmulatorIntegrationTest* --gtest_output=xml:test-results-integration.xml
./VitaNSTests --gtest_filter=PerformanceTest* --gtest_output=xml:test-results-performance.xml
./VitaNSTests --gtest_filter=CompatibilityTest* --gtest_output=xml:test-results-compatibility.xml
```

## Local Development Testing

### **1. Prerequisites**
```bash
# Install devkitPro and Switch toolchain
# Follow: https://devkitpro.org/wiki/Getting_Started

# Install testing dependencies
dkp-pacman -S switch-gtest switch-catch2
```

### **2. Build and Test Commands**
```bash
# Navigate to project directory
cd Vita3K-Switch

# Create build directory
mkdir -p build && cd build

# Configure with Switch toolchain
cmake .. -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake

# Build everything
make -j$(nproc)

# Run specific test categories
make test-renderer      # Unit tests only
make test-integration   # Integration tests only
make test-performance   # Performance tests only
make test-compatibility # Compatibility tests only
make test-all          # All tests

# Direct test execution
./VitaNSTests                           # All tests
./VitaNSTests --gtest_filter=RendererTest*     # Renderer tests only
./VitaNSTests --gtest_filter=PerformanceTest*  # Performance tests only
```

### **3. Test Output and Results**

**Test Results Format:**
```
[==========] Running 15 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 1 test from RendererTest
[ RUN      ] RendererTest.RendererStubInterfaceCompliance
[       OK ] RendererTest.RendererStubInterfaceCompliance (5 ms)
[----------] 1 test from RendererTest (5 ms total)

[----------] Global test environment tear-down
[==========] 15 tests from 1 test suite ran. (25 ms total)
[  PASSED  ] 15 tests.
```

**XML Output (for CI):**
```xml
<?xml version="1.0" encoding="UTF-8"?>
<testsuites tests="15" failures="0" disabled="0" errors="0" time="0.025" timestamp="2024-01-01T00:00:00" name="AllTests">
  <testsuite name="RendererTest" tests="15" failures="0" disabled="0" errors="0" time="0.025">
    <testcase name="RendererStubInterfaceCompliance" status="run" time="0.005" classname="RendererTest"/>
  </testsuite>
</testsuites>
```

## Platform-Specific Considerations

### **1. Switch Hardware Limitations**
- **No GUI**: Tests run headless in CI environment
- **Limited Resources**: Memory and CPU constraints
- **Hardware Emulation**: Some tests may be slower due to emulation overhead

### **2. Test Adaptations for Switch**
```cpp
// Example: Switch-specific test setup
class RendererTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize Switch-specific components
        #ifdef __SWITCH__
        // Switch-specific initialization
        #else
        // PC-specific initialization
        SDL_Init(SDL_INIT_VIDEO);
        #endif
    }
    
    void TearDown() override {
        #ifndef __SWITCH__
        SDL_Quit();
        #endif
    }
};
```

### **3. Performance Expectations**
| Test Category | Switch Target | PC Target | Notes |
|---------------|---------------|-----------|-------|
| Unit Tests | < 30 seconds | < 10 seconds | Switch emulation overhead |
| Integration Tests | < 60 seconds | < 20 seconds | Full system initialization |
| Performance Tests | < 120 seconds | < 40 seconds | Frame rate validation |
| Compatibility Tests | < 90 seconds | < 30 seconds | Game-specific scenarios |

## Debugging and Troubleshooting

### **1. Common Issues**

**GTest Not Found:**
```bash
# Solution: Install Switch GTest
dkp-pacman -S switch-gtest

# Verify installation
ls $DEVKITPRO/portlibs/switch/lib/libgtest*
ls $DEVKITPRO/portlibs/switch/include/gtest/
```

**Test Timeouts:**
```bash
# Increase timeout for specific tests
./VitaNSTests --gtest_filter=PerformanceTest* --gtest_timeout=300000
```

**Memory Issues:**
```bash
# Monitor memory usage during tests
valgrind --tool=memcheck ./VitaNSTests
```

### **2. CI Debugging**

**Check CI Logs:**
```bash
# In GitHub Actions, check for:
echo "=== TEST SUMMARY ==="
./VitaNSTests --gtest_list_tests
echo "=== TEST RESULTS ==="
./VitaNSTests --gtest_brief=1
```

**Artifact Analysis:**
- Download `test-results` artifact
- Analyze XML test results
- Check for specific test failures

### **3. Local vs CI Differences**

| Aspect | Local Development | CI Environment |
|--------|-------------------|----------------|
| Hardware | Native Switch/PC | Docker container |
| Resources | Full system | Limited container |
| GUI | Available | Headless |
| Debugging | Interactive | Log-based |
| Performance | Native speed | Container overhead |

## Best Practices

### **1. Test Development**
- **Write platform-agnostic tests** when possible
- **Use conditional compilation** for platform-specific code
- **Include both positive and negative test cases**
- **Test error conditions and edge cases**

### **2. Performance Testing**
- **Establish baseline metrics** for each platform
- **Use consistent test data** across runs
- **Account for hardware variations** in Switch models
- **Monitor for performance regressions**

### **3. CI Integration**
- **Keep tests fast** to avoid CI timeouts
- **Use appropriate timeouts** for different test categories
- **Generate machine-readable output** (XML format)
- **Upload test artifacts** for analysis

### **4. Documentation**
- **Document test purpose** and expected behavior
- **Include setup instructions** for local development
- **Maintain test coverage** documentation
- **Update this guide** when adding new test categories

## Future Enhancements

### **1. Advanced Testing Features**
- **Automated Switch hardware testing** (when available)
- **Game compatibility database** with automated testing
- **Performance regression detection** with historical data
- **Coverage reporting** for Switch builds

### **2. CI Improvements**
- **Parallel test execution** for faster CI runs
- **Cached dependencies** to speed up builds
- **Automated performance analysis** and reporting
- **Integration with external testing services**

### **3. Developer Experience**
- **Test result visualization** in CI
- **Local test environment** setup scripts
- **Debug test failures** with detailed logging
- **Test development** guidelines and templates

## Conclusion

The VitaNS testing framework provides comprehensive validation for the Nintendo Switch platform through:

- **Automated CI testing** in GitHub Actions
- **Local development testing** with devkitPro
- **Multiple test categories** covering all aspects of the renderer
- **Platform-specific adaptations** for Switch hardware
- **Comprehensive documentation** and debugging support

This ensures that the VitaNS renderer system is thoroughly tested and validated before release, providing confidence in the quality and reliability of the emulator for Switch users. 