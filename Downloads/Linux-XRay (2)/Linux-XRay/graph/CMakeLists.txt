add_library(xray-graph STATIC
    node.cpp
    edge.cpp
    graph.cpp
    layout_engine.cpp
)

target_include_directories(xray-graph PUBLIC
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(xray-graph PUBLIC
    xray-engine
    Threads::Threads
)
