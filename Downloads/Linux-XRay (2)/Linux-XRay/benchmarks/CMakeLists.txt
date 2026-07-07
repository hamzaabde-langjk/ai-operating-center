if(NOT benchmark_FOUND)
    return()
endif()

add_executable(xray-benchmarks
    event_benchmark.cpp
)

target_link_libraries(xray-benchmarks PRIVATE
    xray-engine
    xray-graph
    benchmark::benchmark
    Threads::Threads
)
