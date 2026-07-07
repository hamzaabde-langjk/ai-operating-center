# eBPF programs are compiled separately using clang and llvm
# This CMake file sets up the build rules for BPF objects

if(NOT XRAY_ENABLE_EBPF)
    return()
endif()

find_program(CLANG clang)
find_program(LLVM_STRIP llvm-strip)
find_program(BPFTOOL bpftool)

if(NOT CLANG)
    message(WARNING "clang not found, eBPF programs will not be built")
    return()
endif()

if(NOT BPFTOOL)
    message(WARNING "bpftool not found, eBPF skeletons will not be generated")
endif()

set(BPF_C_FLAGS 
    -g -O2 -target bpf
    -D__TARGET_ARCH_x86_64
    -I${CMAKE_SOURCE_DIR}/kernel
    -I/usr/include
    -I/usr/include/x86_64-linux-gnu
    -I/usr/include/aarch64-linux-gnu
)

set(BPF_SOURCES
    ${CMAKE_SOURCE_DIR}/kernel/process_tracker.bpf.c
    ${CMAKE_SOURCE_DIR}/kernel/file_tracker.bpf.c
    ${CMAKE_SOURCE_DIR}/kernel/network_tracker.bpf.c
)

foreach(src ${BPF_SOURCES})
    get_filename_component(name ${src} NAME_WE)
    set(obj ${CMAKE_BINARY_DIR}/kernel/${name}.o)

    add_custom_command(
        OUTPUT ${obj}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/kernel
        COMMAND ${CLANG} ${BPF_C_FLAGS} -c ${src} -o ${obj}
        COMMAND ${LLVM_STRIP} -g ${obj}
        DEPENDS ${src} ${CMAKE_SOURCE_DIR}/kernel/common_defs.h
        COMMENT "Building eBPF object: ${name}.o"
    )

    list(APPEND BPF_OBJECTS ${obj})
endforeach()

add_custom_target(xray-kernel ALL DEPENDS ${BPF_OBJECTS})

# eBPF loader library
add_library(xray-kernel-loader STATIC
    ebpf_loader.cpp
)

target_include_directories(xray-kernel-loader PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${LIBBPF_INCLUDE_DIRS}
)

target_link_libraries(xray-kernel-loader PUBLIC
    xray-engine
    ${LIBBPF_LIBRARIES}
    elf
    z
)

# Install BPF objects
install(FILES ${BPF_OBJECTS} DESTINATION ${CMAKE_INSTALL_LIBDIR}/xray/bpf)
