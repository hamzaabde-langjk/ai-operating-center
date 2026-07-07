add_executable(xray
    xray.cpp
)

target_link_libraries(xray PRIVATE
    xray-engine
    xray-graph
    xray-database
    xray-security
    spdlog::spdlog
    cxxopts::cxxopts
)

add_executable(xray-top
    xray_top.cpp
)

target_link_libraries(xray-top PRIVATE
    spdlog::spdlog
)

add_executable(xray-monitor
    xray_monitor.cpp
)

target_link_libraries(xray-monitor PRIVATE
    spdlog::spdlog
)

install(TARGETS xray xray-top xray-monitor
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
