add_library(xray-plugins STATIC
    plugin_manager.cpp
)

target_include_directories(xray-plugins PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/sdk/include
)

target_link_libraries(xray-plugins PUBLIC
    xray-engine
    xray-graph
    dl
    spdlog::spdlog
)

# Sample plugin (shared library)
add_library(sample_plugin SHARED
    ${CMAKE_SOURCE_DIR}/sdk/examples/sample_plugin.cpp
)

target_include_directories(sample_plugin PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/sdk/include
)

target_link_libraries(sample_plugin PRIVATE
    xray-plugins
    spdlog::spdlog
)
