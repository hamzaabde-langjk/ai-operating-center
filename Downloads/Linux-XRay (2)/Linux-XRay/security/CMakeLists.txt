add_library(xray-security STATIC
    security_analyzer.cpp
)

target_include_directories(xray-security PUBLIC
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(xray-security PUBLIC
    xray-engine
    xray-graph
    spdlog::spdlog
    Threads::Threads
)
