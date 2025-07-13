# Only add SceGxm if not building for Switch (OpenGL/EGL required)
if(NOT VITANS_PLATFORM_SWITCH)
add_library(SceGxm ${CMAKE_SOURCE_DIR}/src/modules/SceGxm/SceGxm.cpp
                     ${CMAKE_SOURCE_DIR}/src/modules/SceGxm/SceGxm_shader.cpp
                     ${CMAKE_SOURCE_DIR}/src/modules/SceGxm/GxmMemoryManager.cpp)

# Include directories
target_include_directories(SceGxm PRIVATE ${CMAKE_SOURCE_DIR}/src
                                        ${CMAKE_SOURCE_DIR}/src/core
                                        ${CMAKE_SOURCE_DIR}/src/modules/SceGxm
                                        ${CMAKE_SOURCE_DIR}/src/renderer/gl)

# Link libraries
target_link_libraries(SceGxm PRIVATE SwitchRenderer)

# Add shader translation tool
add_executable(ShaderTranslator ${CMAKE_SOURCE_DIR}/src/modules/SceGxm/tools/shader_translator.cpp)
target_link_libraries(ShaderTranslator PRIVATE SceGxm)

# Add shader cache cleanup tool
add_executable(ShaderCacheCleaner ${CMAKE_SOURCE_DIR}/src/modules/SceGxm/tools/shader_cache_cleaner.cpp)
target_link_libraries(ShaderCacheCleaner PRIVATE SceGxm)

# Add shader validation tool
add_executable(ShaderValidator ${CMAKE_SOURCE_DIR}/src/modules/SceGxm/tools/shader_validator.cpp)
target_link_libraries(ShaderValidator PRIVATE SceGxm)
endif()
