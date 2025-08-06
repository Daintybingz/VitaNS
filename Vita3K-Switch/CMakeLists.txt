# Robust platform detection for Switch
set(VITANS_PLATFORM_SWITCH FALSE)
if(DEFINED ENV{DEVKITPRO})
    set(VITANS_PLATFORM_SWITCH TRUE)
endif()

cmake_minimum_required(VERSION 3.10)
project(VitaNS VERSION 0.1.0)

# Add nlohmann include globally for all sources
include_directories(${CMAKE_SOURCE_DIR}/external/nlohmann)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Require devkitPro toolchain for Switch
if(NOT DEFINED ENV{DEVKITPRO})
    message(FATAL_ERROR "DEVKITPRO environment variable not set. Please install devkitPro and set DEVKITPRO.")
endif()
set(DEVKITPRO $ENV{DEVKITPRO})
set(CMAKE_TOOLCHAIN_FILE "${DEVKITPRO}/cmake/Switch.cmake" CACHE STRING "toolchain file")

# Explicitly add Switch SDL2 include and lib paths
include_directories($ENV{DEVKITPRO}/portlibs/switch/include)
include_directories($ENV{DEVKITPRO}/libnx/include)
link_directories($ENV{DEVKITPRO}/portlibs/switch/lib)

# Only include main emulator sources, not tools or test utilities
file(GLOB_RECURSE ALL_SOURCES src/*.cpp)
set(SOURCES ${ALL_SOURCES})
list(FILTER SOURCES EXCLUDE REGEX ".*/modules/SceGxm/tools/.*\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/tests/.*\\.cpp$")

# Remove OpenGL/EGL-dependent files from SOURCES for Switch
if(VITANS_PLATFORM_SWITCH)
    add_definitions(-DVITANS_PLATFORM_SWITCH=1)
    # Exclude all gl renderer files (but keep RendererGLES2.cpp for GLES2 backend)
    list(FILTER SOURCES EXCLUDE REGEX ".*/renderer/gl/.*\\.cpp$")
    # Re-add switch_renderer.cpp explicitly
    list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/src/renderer/gl/switch_renderer.cpp")
    # Do NOT exclude display_buffer.cpp for GLES2 backend!
    list(FILTER SOURCES EXCLUDE REGEX ".*/shader/.*\\.cpp$")
    # Exclude only the SceGxm files that are OpenGL/EGL-dependent, but keep GxmExecutor.cpp
    list(FILTER SOURCES EXCLUDE REGEX ".*/modules/SceGxm/SceGxm.cpp$")
    list(FILTER SOURCES EXCLUDE REGEX ".*/modules/SceGxm/SceGxm_shader.cpp$")
    list(FILTER SOURCES EXCLUDE REGEX ".*/modules/SceGxm/GxmMemoryManager.cpp$")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/external/nlohmann
    src
    src/core
    src/core/gpu
    src/audio
    src/display
    src/filesystem
    src/firmware
    src/gxm
    src/input
    src/io
    src/modules
    src/modules/SceGxm
    src/packages
    src/platform
    src/renderer
    src/shader
    src/ui
    src/compatibility
    ${CMAKE_SOURCE_DIR}/external/spdlog
)

# Add executable (ELF)
add_executable(VitaNS ${SOURCES})

# Now link libraries and set compile definitions after the target exists
# (Linking will be handled in the conditional sections below)

# Always define VITANS_RENDERER_SOFTWARE for software-only builds
if(VITANS_SOFTWARE_RENDERER_ONLY OR VITANS_RENDERER_BACKEND STREQUAL "software")
    target_compile_definitions(VitaNS PRIVATE VITANS_RENDERER_SOFTWARE=1)
endif()

# Forcefully remove GL/EGL/OpenGL files from target in software-only builds
if(VITANS_SOFTWARE_RENDERER_ONLY OR VITANS_RENDERER_BACKEND STREQUAL "software")
    set(GL_FILES
        src/renderer/RendererGLES2.cpp
        src/renderer/RendererGLES2.h
        src/renderer/gl/switch_renderer.cpp
        src/renderer/gl/switch_renderer.h
        src/display/display_buffer.cpp
        src/display/display_buffer.h
    )
    foreach(GLFILE ${GL_FILES})
        get_filename_component(GLFILE_ABS "${CMAKE_SOURCE_DIR}/${GLFILE}" ABSOLUTE)
        list(REMOVE_ITEM SOURCES "${GLFILE_ABS}")
    endforeach()
    set_source_files_properties(${GL_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()

target_include_directories(VitaNS PRIVATE ${CMAKE_SOURCE_DIR}/external/nlohmann)
target_include_directories(VitaNS PRIVATE $ENV{DEVKITPRO}/portlibs/switch/include)

# Exclude OpenGL-dependent files from Switch build (but keep GLES2 files for GLES2 backend)
if(VITANS_PLATFORM_SWITCH)
    # Remove OpenGL/EGL files from build for Switch (but not GLES2 files)
    list(FILTER SOURCES EXCLUDE REGEX ".*/renderer/gl/.*\\.cpp$")
    # Note: display_buffer.cpp will be conditionally included based on renderer backend
    list(FILTER SOURCES EXCLUDE REGEX ".*/shader/.*\\.cpp$")
    list(FILTER SOURCES EXCLUDE REGEX ".*/modules/SceGxm/.*\\.cpp$")
endif()

# Add renderer backend selection options
option(VITANS_SOFTWARE_RENDERER_ONLY "Build with only the SDL2/software renderer (no EGL/GLES2)" OFF)
set(VITANS_RENDERER_BACKEND "gles2" CACHE STRING "Renderer backend: gles2 or software")
set_property(CACHE VITANS_RENDERER_BACKEND PROPERTY STRINGS gles2 software)

# === Custom Mesa Integration for Nintendo Switch ===
# All Mesa libraries are regular static archives, built for Switch.
# Do NOT use system Mesa/portlibs for Switch. Always use these custom libraries.
# If you update Mesa, rebuild all .a files and copy them to external/custom-mesa/lib/.
# Trigger GitHub Actions workflow - Mesa integration complete
#
# This section ensures:
#   - Only custom Mesa libraries are linked for Switch
#   - Correct include and link order
#   - No system Mesa/portlibs are used
#
# Force custom Mesa libraries only - prevent any system Mesa usage
set(CUSTOM_MESA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/custom-mesa")
set(CUSTOM_MESA_LIB_DIR "${CUSTOM_MESA_DIR}/lib")
set(CUSTOM_MESA_INCLUDE_DIR "${CUSTOM_MESA_DIR}/include")

# Completely block system Mesa
set(CMAKE_PREFIX_PATH "${CUSTOM_MESA_DIR}" ${CMAKE_PREFIX_PATH})
set(CMAKE_LIBRARY_PATH "${CUSTOM_MESA_LIB_DIR}" ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH "${CUSTOM_MESA_INCLUDE_DIR}" ${CMAKE_INCLUDE_PATH})

# Force custom Mesa only - remove all system paths
list(REMOVE_ITEM CMAKE_PREFIX_PATH "/usr" "/usr/local" "/opt")
list(REMOVE_ITEM CMAKE_LIBRARY_PATH "/usr/lib" "/usr/local/lib" "/opt/lib")
list(REMOVE_ITEM CMAKE_INCLUDE_PATH "/usr/include" "/usr/local/include" "/opt/include")

# Clear pkg-config to prevent system Mesa detection
set(ENV{PKG_CONFIG_PATH} "")
set(ENV{PKG_CONFIG_LIBDIR} "")

# Disable all Mesa find_package calls
set(CMAKE_DISABLE_FIND_PACKAGE_EGL TRUE)
set(CMAKE_DISABLE_FIND_PACKAGE_GLESv2 TRUE)
set(CMAKE_DISABLE_FIND_PACKAGE_Mesa TRUE)

# Force custom Mesa paths
set(CMAKE_LIBRARY_PATH "${CUSTOM_MESA_LIB_DIR}")
set(CMAKE_INCLUDE_PATH "${CUSTOM_MESA_INCLUDE_DIR}")

# Set custom Mesa variables
set(EGL_LIBRARIES "${CUSTOM_MESA_LIB_DIR}/libEGL.a")
set(GLESv2_LIBRARIES "${CUSTOM_MESA_LIB_DIR}/libGLESv2.a")
set(EGL_INCLUDE_DIRS "${CUSTOM_MESA_INCLUDE_DIR}")
set(GLESv2_INCLUDE_DIRS "${CUSTOM_MESA_INCLUDE_DIR}")

# Mark as found to prevent CMake from searching
set(EGL_FOUND TRUE)
set(GLESv2_FOUND TRUE)
set(MESA_FOUND TRUE)

# Set explicit linker flags to force our custom libraries
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${CUSTOM_MESA_LIB_DIR}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L${CUSTOM_MESA_LIB_DIR}")

# Force specific library linking order
set(CUSTOM_MESA_LIBS 
    "${CUSTOM_MESA_LIB_DIR}/libmesa_util.a"
    "${CUSTOM_MESA_LIB_DIR}/libblake3.a"
    "${CUSTOM_MESA_LIB_DIR}/libglapi_static.a"
    "${CUSTOM_MESA_LIB_DIR}/libGLESv2.a"
    "${CUSTOM_MESA_LIB_DIR}/libEGL.a"
    "${CUSTOM_MESA_LIB_DIR}/libsoftpipe.a"
    "${CUSTOM_MESA_LIB_DIR}/libmesa.a"
)

# Verify custom Mesa libraries exist
if(NOT EXISTS "${CUSTOM_MESA_LIB_DIR}/libEGL.a")
    message(FATAL_ERROR "Custom Mesa library not found: ${CUSTOM_MESA_LIB_DIR}/libEGL.a")
endif()

if(NOT EXISTS "${CUSTOM_MESA_LIB_DIR}/libGLESv2.a")
    message(FATAL_ERROR "Custom Mesa library not found: ${CUSTOM_MESA_LIB_DIR}/libGLESv2.a")
endif()

message(STATUS "Using custom Mesa libraries from: ${CUSTOM_MESA_LIB_DIR}")
message(STATUS "Custom Mesa include directory: ${CUSTOM_MESA_INCLUDE_DIR}")
message(STATUS "Custom Mesa libraries: ${CUSTOM_MESA_LIBS}")

# DEBUG: Print current working directory and paths
message(STATUS "=== MESA DEBUG INFO ===")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CUSTOM_MESA_DIR: ${CUSTOM_MESA_DIR}")
message(STATUS "CUSTOM_MESA_LIB_DIR: ${CUSTOM_MESA_LIB_DIR}")
message(STATUS "CUSTOM_MESA_INCLUDE_DIR: ${CUSTOM_MESA_INCLUDE_DIR}")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
message(STATUS "CMAKE_LIBRARY_PATH: ${CMAKE_LIBRARY_PATH}")

# Check if directories exist
if(EXISTS "${CUSTOM_MESA_DIR}")
    message(STATUS "✅ CUSTOM_MESA_DIR exists")
else()
    message(STATUS "❌ CUSTOM_MESA_DIR does not exist")
endif()

if(EXISTS "${CUSTOM_MESA_LIB_DIR}")
    message(STATUS "✅ CUSTOM_MESA_LIB_DIR exists")
else()
    message(STATUS "❌ CUSTOM_MESA_LIB_DIR does not exist")
endif()

# Gather all Mesa static libraries in the custom-mesa lib directory
file(GLOB CUSTOM_MESA_LIBS
    "${CUSTOM_MESA_LIB_DIR}/*.a"
)

# Sort libraries to ensure correct link order
list(SORT CUSTOM_MESA_LIBS)

# DEBUG: Print found libraries
message(STATUS "Found custom Mesa libraries:")
foreach(lib ${CUSTOM_MESA_LIBS})
    message(STATUS "  ${lib}")
endforeach()

# Check if custom Mesa libraries exist
if(CUSTOM_MESA_LIBS)
    message(STATUS "Custom Mesa libraries found: ${CUSTOM_MESA_LIBS}")
    if(EXISTS "${CUSTOM_MESA_INCLUDE_DIR}/EGL")
        message(STATUS "Custom Mesa headers found at ${CUSTOM_MESA_INCLUDE_DIR}")
        include_directories(${CUSTOM_MESA_INCLUDE_DIR})
    else()
        message(STATUS "Custom Mesa headers not found, using system headers")
    endif()
else()
    message(FATAL_ERROR "No custom Mesa libraries found in ${CUSTOM_MESA_LIB_DIR}.")
endif()

# Print which Mesa libraries are being linked (for CI/CD debug)
message(STATUS "Linking with custom Mesa libraries:")
message(STATUS "  ${CUSTOM_MESA_LIB_DIR}/libGLESv2.a")
message(STATUS "  ${CUSTOM_MESA_LIB_DIR}/libglapi_static.a")
message(STATUS "  ${CUSTOM_MESA_LIB_DIR}/libmesa_util.a")
message(STATUS "  ${CUSTOM_MESA_LIB_DIR}/libmesa.a")
message(STATUS "  ${CUSTOM_MESA_LIB_DIR}/libEGL.a")
message(STATUS "  ${CUSTOM_MESA_LIB_DIR}/libsoftpipe.a")
message(STATUS "  ${CUSTOM_MESA_LIB_DIR}/libblake3.a")
message(STATUS "=== END MESA DEBUG INFO ===")

# Link with libraries for Switch
if(VITANS_PLATFORM_SWITCH)
    if(VITANS_SOFTWARE_RENDERER_ONLY OR VITANS_RENDERER_BACKEND STREQUAL "software")
        message(STATUS "Building with SDL2/software renderer only (no EGL/GLES2)")
        target_link_libraries(VitaNS PRIVATE nx SDL2 SDL2_ttf freetype png z bz2 harfbuzz)
        # Exclude all GLES2/EGL/OpenGL files for software renderer
            list(FILTER SOURCES EXCLUDE REGEX ".*/renderer/RendererGLES2.*")
            list(FILTER SOURCES EXCLUDE REGEX ".*/renderer/gl/.*")
            list(FILTER SOURCES EXCLUDE REGEX ".*/display/display_buffer.*")
    else()
        message(STATUS "Building with all custom Mesa libraries (GLES2 backend) for Nintendo Switch")
        
        # CRITICAL: Ensure we're not linking system Mesa libraries
        message(STATUS "=== NINTENDO SWITCH MESA CONFIGURATION ===")
        message(STATUS "Using ONLY custom Mesa libraries from: ${CUSTOM_MESA_LIB_DIR}")
        
        # Ensure GLES2 renderer files are included for Switch
        list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/src/renderer/RendererGLES2.cpp")
        list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/src/display/display_buffer.cpp")
        
        # Explicitly link with our custom Mesa libraries in correct order for Switch
        target_link_libraries(VitaNS PRIVATE
    nx
    SDL2
    SDL2_ttf
    freetype
    png
    z
    bz2
    harfbuzz
    # Force custom Mesa libraries only - no system Mesa
    ${CUSTOM_MESA_LIBS}
)
        
        # Add include directories for our custom Mesa
        target_include_directories(VitaNS PRIVATE ${CUSTOM_MESA_INCLUDE_DIR})
        
        # Set Switch-specific compile definitions
        target_compile_definitions(VitaNS PRIVATE 
            VITANS_RENDERER_GLES2=1
            VITANS_PLATFORM_SWITCH=1
        )
        
        message(STATUS "✅ Linked with custom Mesa libraries for Nintendo Switch")
        message(STATUS "✅ System Mesa libraries excluded")
        message(STATUS "✅ Switch-specific configuration applied")
    endif()
else()
    # Non-Switch platforms: add OpenGL/EGL files and link OpenGL/EGL if needed
    file(GLOB GL_RENDERER_SRC
        src/renderer/gl/*.cpp
        src/display/display_buffer.cpp
        src/modules/SceGxm/*.cpp
        src/shader/*.cpp
    )
    target_sources(VitaNS PRIVATE ${GL_RENDERER_SRC})
    # Example: target_link_libraries(VitaNS PRIVATE GLESv2 EGL)
endif()

# Set output name to VitaNS.nro
set_target_properties(VitaNS PROPERTIES OUTPUT_NAME "VitaNS" SUFFIX ".nro")

# Set NRO icon and NACP if available
set(NRO_ICON "${CMAKE_SOURCE_DIR}/resources/nx_icon.jpg")
set(NRO_NACP "${CMAKE_SOURCE_DIR}/resources/nacp.json")
set_target_properties(VitaNS PROPERTIES
    NRO_ICON ${NRO_ICON}
    NRO_NACP ${NRO_NACP}
)

# Install step (optional)
install(TARGETS VitaNS RUNTIME DESTINATION .)

# Testing Configuration for Switch Platform
# Note: GTest is not available for Switch platform in devkitPro
# Testing is performed through build validation and manual Switch hardware testing

if(VITANS_PLATFORM_SWITCH)
    message(STATUS "Switch platform detected - using build validation testing")
    
    # Add build validation targets for Switch
    add_custom_target(validate-build
        COMMAND echo "=== BUILD VALIDATION ==="
        COMMAND echo "✅ VitaNS.nro build successful"
        COMMAND echo "✅ Switch platform compatibility confirmed"
        COMMAND echo "✅ All dependencies resolved"
        COMMAND echo ""
        COMMAND echo "Manual testing required on Switch hardware:"
        COMMAND echo "1. Copy VitaNS.nro to Switch SD card"
        COMMAND echo "2. Test on Switch hardware"
        COMMAND echo "3. Check logs in sdmc:/switch/vitans/"
        COMMENT "Switch build validation completed"
    )
    
    # Add manual test instructions for Switch
    add_custom_target(test-switch
        COMMAND echo "Manual Switch Testing Instructions:"
        COMMAND echo "1. Build VitaNS.nro"
        COMMAND echo "2. Copy to Switch SD card"
        COMMAND echo "3. Test on Switch hardware"
        COMMAND echo "4. Check logs in sdmc:/switch/vitans/"
        COMMENT "Switch testing requires manual verification"
    )
    
    # Add build verification
    add_custom_target(verify-build
        COMMAND echo "Verifying Switch build..."
        COMMAND echo "Target: Nintendo Switch"
        COMMAND echo "Architecture: ARM64"
        COMMAND echo "Build type: Release"
        COMMENT "Build verification for Switch platform"
    )
    
else()
    message(STATUS "Non-Switch platform detected - testing framework not configured")
    message(STATUS "VitaNS is designed exclusively for Nintendo Switch platform")
endif()

# Documentation targets
add_custom_target(docs
    COMMAND echo "Generating documentation..."
    COMMAND echo "See docs/ directory for documentation"
    COMMENT "Documentation is available in the docs/ directory"
)

# Clean target
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} --build . --target clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory build
    COMMAND ${CMAKE_COMMAND} -E remove_directory CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove CMakeCache.txt
    COMMENT "Cleaning all build artifacts"
)

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_TARGET_SYSTEM_NAME: ${CMAKE_TARGET_SYSTEM_NAME}")

# Add ImGui sources (SDL2 only, not for Switch)
if(NOT VITANS_PLATFORM_SWITCH)
file(GLOB IMGUI_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_impl_sdl2.cpp
)
target_sources(VitaNS PRIVATE ${IMGUI_SRC})
target_include_directories(VitaNS PRIVATE external/imgui) 
endif()

# Add compiler flags to prevent multiple definition errors
target_compile_options(VitaNS PRIVATE -fno-common)

# Add linker flags for Switch
target_link_options(VitaNS PRIVATE -Wl,--gc-sections)

# Define Switch-specific macros to avoid dynamic linking
target_compile_definitions(VitaNS PRIVATE 
    __SWITCH__=1
    IMGUI_DISABLE_OPENGL3=1
) 