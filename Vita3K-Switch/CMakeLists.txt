cmake_minimum_required(VERSION 3.16)
project(VitaNS)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set custom Mesa directories
set(CUSTOM_MESA_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/custom-mesa/lib")
set(CUSTOM_MESA_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/custom-mesa/include")

# Find required packages - temporarily disable ImGui dependencies
find_package(PkgConfig REQUIRED)
# Temporarily disable to test if ImGui is causing crashes
# pkg_check_modules(SDL2 REQUIRED sdl2)
# pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
# pkg_check_modules(FREETYPE REQUIRED freetype2)
# pkg_check_modules(PNG REQUIRED libpng)
# pkg_check_modules(HARFBUZZ REQUIRED harfbuzz)

# Find Switch libraries
find_library(NX_LIBRARY nx)
find_library(Z_LIBRARY z)
find_library(BZ2_LIBRARY bz2)

# Include directories - prioritize our custom headers
include_directories(
    ${CUSTOM_MESA_INCLUDE_DIR}  # Must come first to override system headers
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    # Temporarily disable ImGui to test if it's causing crashes
    # ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann
    ${CMAKE_CURRENT_SOURCE_DIR}/external/spdlog
    # Temporarily disable ImGui-related include directories
    # ${SDL2_INCLUDE_DIRS}
    # ${SDL2_TTF_INCLUDE_DIRS}
    # ${FREETYPE_INCLUDE_DIRS}
    # ${PNG_INCLUDE_DIRS}
    # ${HARFBUZZ_INCLUDE_DIRS}
)

# Collect only essential source files for no-ImGui test
set(SOURCES 
    "src/main.cpp"
    "src/platform/switch_native_window.cpp"
    # Only include essential platform files - exclude everything else to avoid SDL2 dependencies
)

# Create executable
add_executable(VitaNS ${SOURCES})

# Link libraries with proper Mesa linking using linker groups
target_link_libraries(VitaNS PRIVATE
    nx
    # Temporarily disable some dependencies used by ImGui
    # SDL2
    # SDL2_ttf
    # freetype
    # png
    # z
    # bz2
    # harfbuzz
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

# Set compiler flags and definitions - temporarily disable ImGui flags
target_compile_options(VitaNS PRIVATE
    # Temporarily disable ImGui-related compile flags
    # ${SDL2_CFLAGS_OTHER}
    # ${SDL2_TTF_CFLAGS_OTHER}
    # ${FREETYPE_CFLAGS_OTHER}
    # ${PNG_CFLAGS_OTHER}
    # ${HARFBUZZ_CFLAGS_OTHER}
)

# Add compiler definitions - temporarily removing ImGui flags
target_compile_definitions(VitaNS PRIVATE
    # Temporarily disable ImGui definitions to test if it's causing crashes
    # IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    # IMGUI_DISABLE_DEMO_WINDOWS
    # IMGUI_IMPL_OPENGL_ES2
    # IMGUI_IMPL_OPENGL_LOADER_CUSTOM
    MESA_GLESv2_ENABLE_TLS
    VITANS_RENDERER_SOFTWARE
    VITANS_NO_IMGUI_TEST
)

# Set linker flags - temporarily disable ImGui linker flags
set_target_properties(VitaNS PROPERTIES
    LINK_FLAGS "-Wl,--allow-multiple-definition"
    # Temporarily disabled: "${SDL2_LDFLAGS} ${SDL2_TTF_LDFLAGS} ${FREETYPE_LDFLAGS} ${PNG_LDFLAGS} ${HARFBUZZ_LDFLAGS}"
)

# Set output name for Switch
set_target_properties(VitaNS PROPERTIES
    OUTPUT_NAME "VitaNS"
    SUFFIX ".nro"
)
