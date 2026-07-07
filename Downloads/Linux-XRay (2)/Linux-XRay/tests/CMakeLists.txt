if(NOT GTest_FOUND)
    return()
endif()

add_executable(xray-tests
    test_event_engine.cpp
    test_graph.cpp
)

target_link_libraries(xray-tests PRIVATE
    xray-engine
    xray-graph
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

add_test(NAME xray-unit-tests COMMAND xray-tests)
