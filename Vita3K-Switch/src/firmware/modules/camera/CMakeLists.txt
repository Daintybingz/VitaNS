target_sources(vita3k
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/sceCamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sceCamera_device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sceCamera_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sceCamera_module.cpp
)

target_include_directories(vita3k
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
