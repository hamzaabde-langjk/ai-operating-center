if(NOT SQLite3_FOUND)
    return()
endif()

add_library(xray-database STATIC
    database.cpp
)

target_include_directories(xray-database PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${SQLite3_INCLUDE_DIRS}
)

target_link_libraries(xray-database PUBLIC
    xray-engine
    SQLite::SQLite3
    spdlog::spdlog
)
