cmake_minimum_required(VERSION 3.16)

# Set Switch platform before project definition
set(CMAKE_SYSTEM_NAME NintendoSwitch)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Set Switch toolchain paths
set(CMAKE_C_COMPILER aarch64-none-elf-gcc)
set(CMAKE_CXX_COMPILER aarch64-none-elf-g++)
set(CMAKE_ASM_COMPILER aarch64-none-elf-gcc)

# Set Switch root path
set(CMAKE_FIND_ROOT_PATH /opt/devkitpro/devkitA64/aarch64-none-elf)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Add Switch-specific include paths
include_directories(/opt/devkitpro/libnx/include)

project(VitaNS)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set custom Mesa directories
set(CUSTOM_MESA_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/custom-mesa/lib")
set(CUSTOM_MESA_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/custom-mesa/include")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
pkg_check_modules(FREETYPE REQUIRED freetype2)
pkg_check_modules(PNG REQUIRED libpng)
pkg_check_modules(HARFBUZZ REQUIRED harfbuzz)

# Find Switch libraries
find_library(NX_LIBRARY nx)
find_library(Z_LIBRARY z)
find_library(BZ2_LIBRARY bz2)

# Include directories - prioritize our custom headers
include_directories(
    ${CUSTOM_MESA_INCLUDE_DIR}  # Must come first to override system headers
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann
    ${CMAKE_CURRENT_SOURCE_DIR}/external/spdlog
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${HARFBUZZ_INCLUDE_DIRS}
)

# Collect source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "external/imgui/*.cpp"
)

# Exclude tools and duplicate ImGui sources that cause multiple definitions
list(FILTER SOURCES EXCLUDE REGEX ".*/external/imgui/misc/fonts/binary_to_compressed_c\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/external/imgui/imgui_impl_opengl3\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/external/imgui/imgui_impl_sdl2\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/src/modules/SceGxm/tools/.*\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/src/modules/SceGxm/SceGxm_shader\\.cpp$")

# Exclude test files from main build
list(FILTER SOURCES EXCLUDE REGEX ".*/src/main_.*\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/src/main_.*\\.c$")

# Create executable
add_executable(VitaNS ${SOURCES})

# Create minimal test executable (no EGL/Mesa dependencies)
add_executable(VitaNS_minimal src/main_minimal.cpp)

# Create ultra-minimal test (only basic Switch services)
add_executable(VitaNS_ultra_minimal src/main_ultra_minimal.cpp)

# Create C-only test (bypass C++ runtime entirely)
add_executable(VitaNS_c_only src/main_c_only.c)

# Create super-simple test (no input handling)
add_executable(VitaNS_super_simple src/main_super_simple.cpp)

# Create bare minimum test (just returns 0)
add_executable(VitaNS_bare_minimum src/main_bare_minimum.cpp)

# Link libraries with proper Mesa linking using linker groups
target_link_libraries(VitaNS PRIVATE
    nx
    SDL2
    SDL2_ttf
    freetype
    png
    z
    bz2
    harfbuzz
    # Begin linker group for Mesa static libraries to resolve circular dependencies
    -Wl,--start-group
    # Order: consumers first, providers after
    -Wl,--whole-archive "${CUSTOM_MESA_LIB_DIR}/libEGL.a" -Wl,--no-whole-archive
    "${CUSTOM_MESA_LIB_DIR}/libGLESv2.a"
    "${CUSTOM_MESA_LIB_DIR}/libmesa_util.a"
    "${CUSTOM_MESA_LIB_DIR}/libsoftpipe.a"
    "${CUSTOM_MESA_LIB_DIR}/libblake3.a"
    "${CUSTOM_MESA_LIB_DIR}/libmesa.a"
    "${CUSTOM_MESA_LIB_DIR}/libglapi_static.a"
    -Wl,--end-group
    # System dependencies required by Mesa
    -lpthread
    -lm
)

# Set compiler flags and definitions
target_compile_options(VitaNS PRIVATE
    ${SDL2_CFLAGS_OTHER}
    ${SDL2_TTF_CFLAGS_OTHER}
    ${FREETYPE_CFLAGS_OTHER}
    ${PNG_CFLAGS_OTHER}
    ${HARFBUZZ_CFLAGS_OTHER}
)

# Add compiler definitions to disable dynamic OpenGL loading
target_compile_definitions(VitaNS PRIVATE
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    IMGUI_DISABLE_DEMO_WINDOWS
    IMGUI_IMPL_OPENGL_ES2
    IMGUI_IMPL_OPENGL_LOADER_CUSTOM
    MESA_GLESv2_ENABLE_TLS
    VITANS_RENDERER_SOFTWARE
)

# Set linker flags
set_target_properties(VitaNS PROPERTIES
    LINK_FLAGS "${SDL2_LDFLAGS} ${SDL2_TTF_LDFLAGS} ${FREETYPE_LDFLAGS} ${PNG_LDFLAGS} ${HARFBUZZ_LDFLAGS} -Wl,--allow-multiple-definition"
)

# Set output name for Switch
set_target_properties(VitaNS PROPERTIES
    OUTPUT_NAME "VitaNS"
    SUFFIX ".nro"
)

# Configure minimal test target (basic libnx only)
target_link_libraries(VitaNS_minimal PRIVATE nx)
set_target_properties(VitaNS_minimal PROPERTIES
    OUTPUT_NAME "VitaNS_minimal"
    SUFFIX ".nro"
)

# Configure ultra-minimal test target (only libnx)
target_link_libraries(VitaNS_ultra_minimal PRIVATE nx)
set_target_properties(VitaNS_ultra_minimal PROPERTIES
    OUTPUT_NAME "VitaNS_ultra_minimal"
    SUFFIX ".nro"
)

# Configure C-only test target (only libnx, no C++)
target_link_libraries(VitaNS_c_only PRIVATE nx)
set_target_properties(VitaNS_c_only PROPERTIES
    OUTPUT_NAME "VitaNS_c_only"
    SUFFIX ".nro"
)

# Configure super-simple test target (only libnx, no input)
target_link_libraries(VitaNS_super_simple PRIVATE nx)
set_target_properties(VitaNS_super_simple PROPERTIES
    OUTPUT_NAME "VitaNS_super_simple"
    SUFFIX ".nro"
)

# Configure bare minimum test target (only libnx)
target_link_libraries(VitaNS_bare_minimum PRIVATE nx)
set_target_properties(VitaNS_bare_minimum PROPERTIES
    OUTPUT_NAME "VitaNS_bare_minimum"
    SUFFIX ".nro"
)
