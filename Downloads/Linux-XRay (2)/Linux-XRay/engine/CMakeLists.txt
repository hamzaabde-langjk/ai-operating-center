add_library(xray-engine STATIC
    event.cpp
    event_bus.cpp
    event_dispatcher.cpp
    event_filter.cpp
    timestamp_engine.cpp
    serializer.cpp
    cache.cpp
)

target_include_directories(xray-engine PUBLIC
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(xray-engine PUBLIC
    spdlog::spdlog
    Threads::Threads
)

if(XRAY_ENABLE_EBPF)
    target_compile_definitions(xray-engine PUBLIC XRAY_ENABLE_EBPF)
endif()
