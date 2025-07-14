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
    # Exclude all gl renderer files
    list(FILTER SOURCES EXCLUDE REGEX ".*/renderer/gl/.*\\.cpp$")
    # Re-add switch_renderer.cpp explicitly
    list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/src/renderer/gl/switch_renderer.cpp")
    # Do NOT exclude display_buffer.cpp!
    list(FILTER SOURCES EXCLUDE REGEX ".*/shader/.*\\.cpp$")
    list(FILTER SOURCES EXCLUDE REGEX ".*/modules/SceGxm/.*\\.cpp$")
endif()

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

target_include_directories(VitaNS PRIVATE ${CMAKE_SOURCE_DIR}/external/nlohmann)
target_include_directories(VitaNS PRIVATE $ENV{DEVKITPRO}/portlibs/switch/include)

# Exclude OpenGL-dependent files from Switch build
if(VITANS_PLATFORM_SWITCH)
    # Remove OpenGL/EGL files from build for Switch
    list(FILTER SOURCES EXCLUDE REGEX ".*/renderer/gl/.*\\.cpp$")
    list(FILTER SOURCES EXCLUDE REGEX ".*/display/display_buffer\\.cpp$")
    list(FILTER SOURCES EXCLUDE REGEX ".*/shader/.*\\.cpp$")
    list(FILTER SOURCES EXCLUDE REGEX ".*/modules/SceGxm/.*\\.cpp$")
endif()

# Link with only SDL2, SDL2_ttf, freetype, png, z, bz2 for Switch
if(VITANS_PLATFORM_SWITCH)
    target_link_libraries(VitaNS PRIVATE nx SDL2 SDL2_ttf freetype png z bz2 EGL GLESv2 harfbuzz glapi)
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

# Testing Configuration
# Enable testing for both Switch and non-Switch platforms
# Find GTest (available in devkitPro for Switch)
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    # Try to find GTest in devkitPro paths for Switch
    if(VITANS_PLATFORM_SWITCH)
        find_path(GTEST_INCLUDE_DIR gtest/gtest.h
            PATHS $ENV{DEVKITPRO}/portlibs/switch/include
            NO_DEFAULT_PATH
        )
        find_library(GTEST_LIBRARY gtest
            PATHS $ENV{DEVKITPRO}/portlibs/switch/lib
            NO_DEFAULT_PATH
        )
        find_library(GTEST_MAIN_LIBRARY gtest_main
            PATHS $ENV{DEVKITPRO}/portlibs/switch/lib
            NO_DEFAULT_PATH
        )
        if(GTEST_INCLUDE_DIR AND GTEST_LIBRARY AND GTEST_MAIN_LIBRARY)
            set(GTest_FOUND TRUE)
            set(GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIR})
            set(GTEST_LIBRARIES ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})
        endif()
    endif()
endif()

if(GTest_FOUND)
    message(STATUS "GTest found - enabling testing framework")
    
    # Test source files
    file(GLOB TEST_SOURCES tests/*.cpp)
    
    if(TEST_SOURCES)
        # Create test executable
        add_executable(VitaNSTests ${TEST_SOURCES})
        
        # Include directories for tests
        target_include_directories(VitaNSTests PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/external/nlohmann
            ${CMAKE_SOURCE_DIR}/external/spdlog
        )
        
        # Link test executable with dependencies
        if(VITANS_PLATFORM_SWITCH)
            # Switch-specific test linking
            target_link_libraries(VitaNSTests PRIVATE 
                nx 
                SDL2 
                SDL2_ttf 
                freetype 
                png 
                z 
                bz2 
                EGL 
                GLESv2 
                harfbuzz 
                glapi
                ${GTEST_LIBRARIES}
            )
            target_include_directories(VitaNSTests PRIVATE $ENV{DEVKITPRO}/portlibs/switch/include)
        else()
            # Non-Switch test linking
            target_link_libraries(VitaNSTests PRIVATE 
                SDL2 
                SDL2_ttf
                GTest::GTest 
                GTest::Main
            )
        endif()
        
        # Add tests to CTest
        add_test(NAME RendererTests COMMAND VitaNSTests --gtest_filter=RendererTest*)
        add_test(NAME IntegrationTests COMMAND VitaNSTests --gtest_filter=EmulatorIntegrationTest*)
        add_test(NAME PerformanceTests COMMAND VitaNSTests --gtest_filter=PerformanceTest*)
        add_test(NAME CompatibilityTests COMMAND VitaNSTests --gtest_filter=CompatibilityTest*)
        
        # Set test properties with platform-specific timeouts
        if(VITANS_PLATFORM_SWITCH)
            # Switch tests may take longer due to hardware emulation
            set_tests_properties(RendererTests PROPERTIES TIMEOUT 600)
            set_tests_properties(IntegrationTests PROPERTIES TIMEOUT 1200)
            set_tests_properties(PerformanceTests PROPERTIES TIMEOUT 1800)
            set_tests_properties(CompatibilityTests PROPERTIES TIMEOUT 900)
        else()
            # Non-Switch tests with standard timeouts
            set_tests_properties(RendererTests PROPERTIES 
                TIMEOUT 300
                ENVIRONMENT "SDL_VIDEODRIVER=dummy"
            )
            set_tests_properties(IntegrationTests PROPERTIES 
                TIMEOUT 600
                ENVIRONMENT "SDL_VIDEODRIVER=dummy"
            )
            set_tests_properties(PerformanceTests PROPERTIES 
                TIMEOUT 1200
                ENVIRONMENT "SDL_VIDEODRIVER=dummy"
            )
            set_tests_properties(CompatibilityTests PROPERTIES 
                TIMEOUT 900
                ENVIRONMENT "SDL_VIDEODRIVER=dummy"
            )
        endif()
        
        # Custom test targets
        add_custom_target(test-renderer
            COMMAND ${CMAKE_COMMAND} --build . --target VitaNSTests
            COMMAND ./VitaNSTests --gtest_filter=RendererTest*
            DEPENDS VitaNSTests
            COMMENT "Running renderer unit tests"
        )
        
        add_custom_target(test-integration
            COMMAND ${CMAKE_COMMAND} --build . --target VitaNSTests
            COMMAND ./VitaNSTests --gtest_filter=EmulatorIntegrationTest*
            DEPENDS VitaNSTests
            COMMENT "Running integration tests"
        )
        
        add_custom_target(test-performance
            COMMAND ${CMAKE_COMMAND} --build . --target VitaNSTests
            COMMAND ./VitaNSTests --gtest_filter=PerformanceTest*
            DEPENDS VitaNSTests
            COMMENT "Running performance tests"
        )
        
        add_custom_target(test-compatibility
            COMMAND ${CMAKE_COMMAND} --build . --target VitaNSTests
            COMMAND ./VitaNSTests --gtest_filter=CompatibilityTest*
            DEPENDS VitaNSTests
            COMMENT "Running compatibility tests"
        )
        
        add_custom_target(test-all
            COMMAND ${CMAKE_COMMAND} --build . --target VitaNSTests
            COMMAND ./VitaNSTests
            DEPENDS VitaNSTests
            COMMENT "Running all tests"
        )
        
        # Coverage configuration (non-Switch only)
        if(NOT VITANS_PLATFORM_SWITCH AND CMAKE_BUILD_TYPE STREQUAL "Debug")
            find_package(gcov)
            if(gcov_FOUND)
                target_compile_options(VitaNSTests PRIVATE --coverage)
                target_link_libraries(VitaNSTests PRIVATE gcov)
                
                add_custom_target(coverage
                    COMMAND ${CMAKE_COMMAND} --build . --target VitaNSTests
                    COMMAND ./VitaNSTests
                    COMMAND gcovr --root . --html --html-details --output coverage.html
                    DEPENDS VitaNSTests
                    COMMENT "Generating coverage report"
                )
            endif()
        endif()
        
    else()
        message(STATUS "No test sources found in tests/ directory")
    endif()
    
else()
    message(STATUS "GTest not found - skipping testing framework")
    
    if(VITANS_PLATFORM_SWITCH)
        # Add manual test instructions for Switch
        add_custom_target(test-switch
            COMMAND echo "Manual Switch Testing Instructions:"
            COMMAND echo "1. Build VitaNS.nro"
            COMMAND echo "2. Copy to Switch SD card"
            COMMAND echo "3. Test on Switch hardware"
            COMMAND echo "4. Check logs in sdmc:/switch/vitans/"
            COMMENT "Switch testing requires manual verification"
        )
    endif()
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