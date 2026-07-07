if(NOT TARGET Qt6::OpenGLWidgets OR NOT TARGET OpenGL::GL)
    return()
endif()

add_library(xray-renderer STATIC
    shader.cpp
    camera.cpp
    scene.cpp
    gl_renderer.cpp
)

target_include_directories(xray-renderer PUBLIC
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(xray-renderer PUBLIC
    xray-graph
    Qt6::OpenGLWidgets
    OpenGL::GL
    glm::glm
    spdlog::spdlog
)
