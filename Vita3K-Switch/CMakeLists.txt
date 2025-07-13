cmake_minimum_required(VERSION 3.10)
project(VitaNS VERSION 0.1.0)

# Add nlohmann include globally for all sources
include_directories(${CMAKE_SOURCE_DIR}/external/nlohmann)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Exclude tool and test sources
set(SOURCES ${ALL_SOURCES})
list(FILTER SOURCES EXCLUDE REGEX ".*/modules/SceGxm/tools/.*\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/tests/.*\\.cpp$")

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/external/nlohmann
    src
    src/core
    src/audio
    src/display
    src/filesystem
    src/firmware
    src/gxm
    src/input
    src/io
    src/modules
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

target_include_directories(VitaNS PRIVATE ${CMAKE_SOURCE_DIR}/external/nlohmann)

# Link with libnx, SDL2, SDL2_ttf, and OpenGL ES for Switch
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
target_link_libraries(VitaNS nx SDL2::SDL2 SDL2_ttf::SDL2_ttf GLESv2)

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

# Add post-build step to convert ELF to NRO using elf2nro
add_custom_command(TARGET VitaNS POST_BUILD
    COMMAND elf2nro $<TARGET_FILE:VitaNS> $<TARGET_FILE_DIR:VitaNS>/VitaNS.nro
        --icon ${NRO_ICON} --nacp ${NRO_NACP}
    COMMENT "Converting VitaNS.elf to VitaNS.nro using elf2nro"
)

# Add ImGui sources (SDL2 only)
file(GLOB IMGUI_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_impl_sdl2.cpp
)
target_sources(VitaNS PRIVATE ${IMGUI_SRC})
target_include_directories(VitaNS PRIVATE external/imgui)

# Add compiler flags to prevent multiple definition errors
target_compile_options(VitaNS PRIVATE -fno-common)

# Add linker flags for Switch
target_link_options(VitaNS PRIVATE -Wl,--gc-sections)

# Define Switch-specific macros to avoid dynamic linking
target_compile_definitions(VitaNS PRIVATE 
    __SWITCH__=1
    IMGUI_DISABLE_OPENGL3=1
) 