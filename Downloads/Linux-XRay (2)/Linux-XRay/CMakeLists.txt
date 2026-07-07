cmake_minimum_required(VERSION 3.25)
project(LinuxXRay VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(XRAY_BUILD_TESTS "Build tests" ON)
option(XRAY_BUILD_BENCHMARKS "Build benchmarks" ON)
option(XRAY_BUILD_GUI "Build Qt6 GUI" ON)
option(XRAY_ENABLE_EBPF "Enable eBPF kernel tracing" ON)
option(XRAY_ENABLE_AI "Enable Python AI integration" ON)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual
        -Wconversion -Wsign-conversion -Wnull-dereference
        -Wdouble-promotion -Wformat=2 -Wimplicit-fallthrough
    )
endif()

# Find packages
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

# spdlog
find_package(spdlog REQUIRED)

# cxxopts (header-only, try find or fetch)
find_package(cxxopts QUIET)
if(NOT cxxopts_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG v3.1.1
    )
    FetchContent_MakeAvailable(cxxopts)
endif()

# glm (header-only)
find_package(glm QUIET)
if(NOT glm_FOUND)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
    )
    FetchContent_MakeAvailable(glm)
endif()

# Qt6
if(XRAY_BUILD_GUI)
    find_package(Qt6 QUIET COMPONENTS Core Widgets OpenGLWidgets)
    if(Qt6_FOUND)
        qt_standard_project_setup()
    else()
        message(WARNING "Qt6 not found, GUI will not be built")
        set(XRAY_BUILD_GUI OFF)
    endif()
endif()

# OpenGL
find_package(OpenGL QUIET)
if(NOT OpenGL_FOUND)
    message(WARNING "OpenGL not found, renderer will be limited")
endif()

# SQLite3
find_package(SQLite3 QUIET)
if(NOT SQLite3_FOUND)
    message(WARNING "SQLite3 not found, database features disabled")
endif()

# eBPF / libbpf
if(XRAY_ENABLE_EBPF)
    pkg_check_modules(LIBBPF libbpf)
    if(NOT LIBBPF_FOUND)
        message(WARNING "libbpf not found, eBPF features disabled")
        set(XRAY_ENABLE_EBPF OFF)
    endif()
endif()

# GoogleTest
if(XRAY_BUILD_TESTS)
    find_package(GTest QUIET)
    if(GTest_FOUND)
        enable_testing()
    else()
        message(WARNING "GoogleTest not found, tests disabled")
        set(XRAY_BUILD_TESTS OFF)
    endif()
endif()

# Benchmark
if(XRAY_BUILD_BENCHMARKS)
    find_package(benchmark QUIET)
    if(NOT benchmark_FOUND)
        message(WARNING "benchmark not found, benchmarks disabled")
        set(XRAY_BUILD_BENCHMARKS OFF)
    endif()
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/third_party)

# Subdirectories
add_subdirectory(engine)
add_subdirectory(graph)

if(XRAY_ENABLE_EBPF AND LIBBPF_FOUND)
    add_subdirectory(kernel)
endif()

if(SQLite3_FOUND)
    add_subdirectory(database)
endif()

add_subdirectory(security)
add_subdirectory(plugins)
add_subdirectory(cli)

if(XRAY_BUILD_GUI AND Qt6_FOUND AND OpenGL_FOUND)
    add_subdirectory(renderer)
    add_subdirectory(gui)
endif()

if(XRAY_BUILD_TESTS AND GTest_FOUND)
    add_subdirectory(tests)
endif()

if(XRAY_BUILD_BENCHMARKS AND benchmark_FOUND)
    add_subdirectory(benchmarks)
endif()

# Install targets
include(GNUInstallDirs)
install(TARGETS xray xray-top xray-monitor
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# CPack
set(CPACK_PACKAGE_NAME "LinuxXRay")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Linux X-Ray Vision - System Monitor")
include(CPack)
