target_sources(vita3k
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/scePower.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scePower_module.cpp
)

target_include_directories(vita3k
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
