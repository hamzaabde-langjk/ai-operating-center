# Common CMake utilities

function(xray_add_test target)
    add_test(NAME ${target} COMMAND ${target})
endfunction()
