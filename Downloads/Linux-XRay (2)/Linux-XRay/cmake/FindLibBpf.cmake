# Find libbpf library and headers
find_path(LIBBPF_INCLUDE_DIRS bpf/libbpf.h
    PATHS /usr/include /usr/local/include
)

find_library(LIBBPF_LIBRARIES NAMES bpf
    PATHS /usr/lib /usr/local/lib /usr/lib64
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibBpf DEFAULT_MSG
    LIBBPF_LIBRARIES LIBBPF_INCLUDE_DIRS)

mark_as_advanced(LIBBPF_INCLUDE_DIRS LIBBPF_LIBRARIES)
