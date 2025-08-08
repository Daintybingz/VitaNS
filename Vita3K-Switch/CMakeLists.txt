cmake_minimum_required(VERSION 3.16)
project(VitaNS)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set custom Mesa library directory
set(CUSTOM_MESA_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/custom-mesa")
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

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann
    ${CMAKE_CURRENT_SOURCE_DIR}/external/spdlog
    ${CUSTOM_MESA_INCLUDE_DIR}
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

# Create executable
add_executable(VitaNS ${SOURCES})

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
    "${CUSTOM_MESA_LIB_DIR}/libEGL.a"
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

# Set compiler flags
target_compile_options(VitaNS PRIVATE
    ${SDL2_CFLAGS_OTHER}
    ${SDL2_TTF_CFLAGS_OTHER}
    ${FREETYPE_CFLAGS_OTHER}
    ${PNG_CFLAGS_OTHER}
    ${HARFBUZZ_CFLAGS_OTHER}
)

# Set linker flags
set_target_properties(VitaNS PROPERTIES
    LINK_FLAGS "${SDL2_LDFLAGS} ${SDL2_TTF_LDFLAGS} ${FREETYPE_LDFLAGS} ${PNG_LDFLAGS} ${HARFBUZZ_LDFLAGS}"
)

# Set output name for Switch
set_target_properties(VitaNS PROPERTIES
    OUTPUT_NAME "VitaNS"
    SUFFIX ".nro"
)
