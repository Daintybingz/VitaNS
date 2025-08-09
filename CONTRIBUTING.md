# Contributing to VitaNS

Thank you for your interest in contributing to VitaNS! This guide will help you get started with contributing to the project.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Development Environment](#development-environment)
3. [Code Style Guidelines](#code-style-guidelines)
4. [Contribution Workflow](#contribution-workflow)
5. [Types of Contributions](#types-of-contributions)
6. [Testing Guidelines](#testing-guidelines)
7. [Documentation Standards](#documentation-standards)
8. [Community Guidelines](#community-guidelines)

## Getting Started

### Prerequisites

Before contributing, ensure you have:
- Read our [README.md](README.md) and [TECHNICAL_GUIDE.md](TECHNICAL_GUIDE.md)
- Set up the development environment following [BUILD_GUIDE.md](BUILD_GUIDE.md)
- Familiarize yourself with the VitaNS architecture
- A GitHub account and basic Git knowledge

### First Steps

1. **Fork the Repository**
   ```bash
   # Fork VitaNS on GitHub, then clone your fork
   git clone https://github.com/YOUR_USERNAME/VitaNS.git
   cd VitaNS
   
   # Add upstream remote
   git remote add upstream https://github.com/original-owner/VitaNS.git
   ```

2. **Verify Build Environment**
   ```bash
   # Ensure you can build VitaNS successfully
   cd Vita3K-Switch
   mkdir build && cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake
   make -j$(nproc)
   ```

3. **Run Tests**
   ```bash
   # Verify basic functionality
   ls VitaNS.nro  # Should exist and be ~56MB
   ```

## Development Environment

### Required Tools

- **DevkitPro**: Nintendo Switch toolchain
- **CMake**: 3.16 or later
- **Git**: Version control with LFS support
- **Code Editor**: VS Code recommended with C++ extensions

### Recommended VS Code Extensions

```bash
# Install useful extensions
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools
code --install-extension ms-vscode-remote.remote-wsl
code --install-extension ms-vscode.hexeditor
```

### Environment Configuration

```json
// .vscode/settings.json
{
    "C_Cpp.default.compilerPath": "/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc",
    "C_Cpp.default.includePath": [
        "/opt/devkitpro/libnx/include",
        "${workspaceFolder}/Vita3K-Switch/external/custom-mesa/include",
        "${workspaceFolder}/Vita3K-Switch/src"
    ],
    "C_Cpp.default.cppStandard": "c++20",
    "cmake.configureArgs": [
        "-DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/Switch.cmake"
    ],
    "files.trimTrailingWhitespace": true,
    "files.insertFinalNewline": true
}
```

## Code Style Guidelines

### C++ Style

We follow a modified version of the Google C++ Style Guide:

#### File Organization
```cpp
// Header file structure
#pragma once

// System includes
#include <memory>
#include <vector>

// Third-party includes  
#include <switch.h>
#include <GLES2/gl2.h>

// Project includes
#include "emulator/emulator.h"
#include "gpu/gpu_subsystem.h"

namespace vitans {
    // Class definitions
}
```

#### Naming Conventions

```cpp
// Classes: PascalCase
class GpuSubsystem {
public:
    // Public methods: camelCase
    void initialize();
    bool isInitialized() const;
    
    // Public members: snake_case (avoid public members)
    // uint32_t frame_count;  // Avoid this
    
private:
    // Private methods: camelCase
    void initializeRenderer();
    
    // Private members: snake_case with underscore suffix
    std::unique_ptr<Renderer> renderer_;
    bool is_initialized_;
};

// Functions: camelCase
void renderFrame();
bool checkGpuStatus();

// Constants: UPPER_SNAKE_CASE
const uint32_t MAX_RENDER_TARGETS = 8;
const char* DEFAULT_SHADER_PATH = "shaders/";

// Enums: PascalCase with enum class
enum class RenderMode {
    Software,
    Hardware,
    Hybrid
};
```

#### Code Formatting

```cpp
// Indentation: 4 spaces (no tabs)
class ExampleClass {
public:
    // Function formatting
    void longFunctionName(
        const std::string& parameter_one,
        uint32_t parameter_two,
        bool parameter_three) {
        
        // Braces on new line for functions, same line for control flow
        if (parameter_three) {
            // Control flow indented 4 spaces
            doSomething();
        }
        
        // Long expressions split logically
        auto result = some_long_function_call(
            parameter_one,
            parameter_two ? "true_value" : "false_value");
    }
    
private:
    // Member initialization
    std::unique_ptr<Renderer> renderer_ = nullptr;
    std::vector<RenderTarget> targets_;
};
```

### Header Guards and Includes

```cpp
// Use #pragma once instead of include guards
#pragma once

// Include order:
// 1. Corresponding header (for .cpp files)
// 2. System headers
// 3. Third-party headers  
// 4. Project headers

#include "gpu_subsystem.h"  // Corresponding header

#include <memory>           // System headers
#include <vector>

#include <switch.h>         // Third-party headers
#include <GLES2/gl2.h>

#include "renderer/renderer_gles2.h"  // Project headers
#include "command/command_buffer.h"
```

### Documentation

```cpp
/**
 * @brief Manages the GPU subsystem for PS Vita graphics emulation
 * 
 * The GpuSubsystem translates PS Vita GXM commands to OpenGL ES calls
 * that can be executed on the Nintendo Switch. It maintains render state,
 * command buffers, and coordinates with the emulated CPU.
 */
class GpuSubsystem {
public:
    /**
     * @brief Initialize the GPU subsystem
     * @param renderer The renderer implementation to use
     * @return true if initialization succeeded
     */
    bool initialize(std::unique_ptr<Renderer> renderer);
    
    /**
     * @brief Submit a command buffer for execution
     * @param commands Vector of GXM commands to execute
     * @param priority Execution priority (0 = highest)
     * @note Commands are executed asynchronously
     */
    void submitCommands(
        const std::vector<GxmCommand>& commands,
        uint32_t priority = 0);
        
private:
    std::unique_ptr<Renderer> renderer_;  ///< Active renderer instance
    CommandQueue command_queue_;          ///< Pending command queue
};
```

## Contribution Workflow

### 1. Create a Feature Branch

```bash
# Update your fork
git fetch upstream
git checkout master
git merge upstream/master

# Create feature branch
git checkout -b feature/your-feature-name
# or
git checkout -b fix/bug-description
```

### 2. Make Changes

```cpp
// Example: Adding a new GXM command
// File: src/modules/SceGxm/SceGxm.cpp

SceInt32 sceGxmNewCommand(SceGxmContext* context, SceGxmCommandType type) {
    if (!context) {
        LOG_ERROR("Invalid context passed to sceGxmNewCommand");
        return SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Validate command type
    if (type >= SCE_GXM_COMMAND_TYPE_MAX) {
        LOG_ERROR("Invalid command type: {}", static_cast<uint32_t>(type));
        return SCE_GXM_ERROR_INVALID_VALUE;
    }
    
    // Create command
    auto cmd = std::make_unique<GxmNewCommand>();
    cmd->type = type;
    
    // Submit to GPU subsystem
    auto& gpu = Emulator::getInstance().getGpuSubsystem();
    gpu.getCommandBuffer().add(std::move(cmd));
    
    return SCE_OK;
}
```

### 3. Write Tests

```cpp
// File: tests/modules/test_sce_gxm.cpp
#include "gtest/gtest.h"
#include "modules/SceGxm/SceGxm.h"

class SceGxmTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test environment
        context_ = createTestContext();
    }
    
    void TearDown() override {
        // Cleanup
        destroyTestContext(context_);
    }
    
    SceGxmContext* context_;
};

TEST_F(SceGxmTest, NewCommandValidInput) {
    SceInt32 result = sceGxmNewCommand(context_, SCE_GXM_COMMAND_TYPE_DRAW);
    EXPECT_EQ(result, SCE_OK);
}

TEST_F(SceGxmTest, NewCommandInvalidContext) {
    SceInt32 result = sceGxmNewCommand(nullptr, SCE_GXM_COMMAND_TYPE_DRAW);
    EXPECT_EQ(result, SCE_GXM_ERROR_INVALID_POINTER);
}
```

### 4. Build and Test

```bash
# Build your changes
cd Vita3K-Switch/build
make -j$(nproc)

# Run tests if available
make test

# Verify output
ls VitaNS.nro
du -h VitaNS.nro
```

### 5. Commit Changes

```bash
# Stage changes
git add .

# Commit with descriptive message
git commit -m "modules/SceGxm: Add sceGxmNewCommand implementation

- Implement basic command creation and validation
- Add error handling for invalid inputs
- Submit commands to GPU subsystem
- Add unit tests for functionality

Fixes #123"
```

### 6. Push and Create Pull Request

```bash
# Push to your fork
git push origin feature/your-feature-name

# Create pull request on GitHub
# Include description of changes, testing performed, and any issues
```

## Types of Contributions

### 1. Core Emulation

**Areas:**
- CPU emulation improvements
- Memory management optimization
- System call implementation
- Module function emulation

**Example:**
```cpp
// Implementing a new SceKernel function
SceUID sceKernelCreateThread(const char* name, SceKernelThreadEntry entry, 
                            int init_priority, SceSize stack_size, 
                            SceUInt attr, int cpu_affinity_mask, 
                            const SceKernelThreadOptParam* option) {
    // Implementation
}
```

### 2. Graphics System

**Areas:**
- GXM command translation
- OpenGL ES optimization
- Mesa integration improvements
- Shader compilation

**Example:**
```cpp
// Adding support for a new texture format
void RendererGLES2::uploadTexture(const GxmTexture& texture) {
    GLenum gl_format = translateTextureFormat(texture.format);
    // Upload logic
}
```

### 3. Platform Integration

**Areas:**
- Switch-specific optimizations
- Input handling improvements
- Audio system implementation
- File system integration

### 4. Testing and Quality Assurance

**Areas:**
- Unit tests
- Integration tests
- Performance benchmarks
- Game compatibility testing

### 5. Documentation

**Areas:**
- Code documentation
- User guides
- Technical documentation
- API reference

## Testing Guidelines

### Unit Tests

```cpp
// Use Google Test framework
#include "gtest/gtest.h"

class MemoryManagerTest : public ::testing::Test {
    // Test setup and teardown
};

TEST_F(MemoryManagerTest, AllocateValidSize) {
    void* ptr = memory_manager_.allocate(1024);
    ASSERT_NE(ptr, nullptr);
    EXPECT_TRUE(memory_manager_.isValidPointer(ptr));
    memory_manager_.deallocate(ptr);
}
```

### Integration Tests

```cpp
// Test full emulation flow
TEST(EmulationTest, BasicGameLoading) {
    Emulator emulator;
    ASSERT_TRUE(emulator.initialize());
    
    bool result = emulator.loadGame("test_game.vpk");
    EXPECT_TRUE(result);
    
    // Run for a few frames
    for (int i = 0; i < 60; ++i) {
        emulator.step();
    }
    
    EXPECT_TRUE(emulator.isRunning());
}
```

### Performance Tests

```cpp
// Benchmark critical paths
#include "benchmark/benchmark.h"

static void BM_GxmCommandExecution(benchmark::State& state) {
    GpuSubsystem gpu;
    gpu.initialize();
    
    for (auto _ : state) {
        // Benchmark command execution
        gpu.executeCommand(createTestCommand());
    }
}
BENCHMARK(BM_GxmCommandExecution);
```

## Documentation Standards

### Code Comments

```cpp
// Good comments explain WHY, not WHAT
void GpuSubsystem::flushCommandBuffer() {
    // Flush immediately to maintain frame timing consistency
    // PS Vita games expect commands to complete within 16ms
    command_buffer_.executeAll();
    
    // Clear after execution to prevent memory buildup
    command_buffer_.clear();
}
```

### API Documentation

```cpp
/**
 * @brief Execute a batch of GXM commands
 * 
 * Processes a list of GXM commands in order, translating them to
 * appropriate OpenGL ES calls. Commands are executed synchronously
 * to maintain proper render state.
 * 
 * @param commands List of commands to execute
 * @param context Current GXM context
 * @return Number of commands successfully executed
 * 
 * @note This function may block if the GPU is busy
 * @warning Commands must be valid and context must be active
 * 
 * @example
 * ```cpp
 * std::vector<GxmCommand> commands = buildRenderCommands();
 * size_t executed = gpu.executeCommands(commands, current_context);
 * ```
 */
size_t executeCommands(const std::vector<GxmCommand>& commands,
                      SceGxmContext* context);
```

## Community Guidelines

### Code of Conduct

- **Be respectful**: Treat all contributors with respect
- **Be constructive**: Provide helpful feedback and suggestions
- **Be patient**: Remember that everyone is learning
- **Be inclusive**: Welcome newcomers and diverse perspectives

### Communication

- **GitHub Issues**: Bug reports and feature requests
- **Pull Requests**: Code changes and discussions
- **Discord**: Real-time chat and community support
- **Discussions**: Design discussions and questions

### Pull Request Guidelines

1. **Clear Description**: Explain what changes and why
2. **Small Focused Changes**: One feature/fix per PR
3. **Tests Included**: Add tests for new functionality
4. **Documentation Updated**: Keep docs in sync with code
5. **Clean History**: Squash commits if needed

### Example Pull Request Template

```markdown
## Description
Brief description of changes made.

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Unit tests pass
- [ ] Integration tests pass
- [ ] Manual testing performed

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] No breaking changes without discussion
```

## Getting Help

### Resources
- [Technical Guide](TECHNICAL_GUIDE.md) - Deep technical details
- [Build Guide](BUILD_GUIDE.md) - Setup and compilation
- [Troubleshooting](TROUBLESHOOTING.md) - Common issues

### Community Support
- **GitHub Discussions**: Design questions and help
- **Discord**: Real-time community support
- **Issues**: Bug reports and feature requests

### Mentorship
New contributors are welcome! Core team members are available to help with:
- Understanding the codebase
- Choosing good first issues
- Code review and feedback
- Technical guidance

---

Thank you for contributing to VitaNS! Your efforts help bring PlayStation Vita gaming to the Nintendo Switch community.
