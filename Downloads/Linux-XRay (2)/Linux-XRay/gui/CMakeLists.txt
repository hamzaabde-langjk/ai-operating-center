if(NOT TARGET Qt6::Core OR NOT TARGET Qt6::Widgets)
    return()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_library(xray-gui STATIC
    main_window.cpp
    dashboard.cpp
    process_view.cpp
    inspector_panel.cpp
    timeline_widget.cpp
    file_view.cpp
    network_view.cpp
    3d_viewport.cpp
)

target_include_directories(xray-gui PUBLIC
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(xray-gui PUBLIC
    xray-engine
    xray-graph
    xray-renderer
    xray-database
    xray-security
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGLWidgets
    spdlog::spdlog
)

# Main executable
add_executable(xray-gui-app
    main.cpp
)

target_link_libraries(xray-gui-app PRIVATE
    xray-gui
    Qt6::Core
    Qt6::Widgets
)

set_target_properties(xray-gui-app PROPERTIES
    OUTPUT_NAME "xray-gui"
)
