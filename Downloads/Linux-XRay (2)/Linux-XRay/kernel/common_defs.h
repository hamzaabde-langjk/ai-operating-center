#ifndef XRAY_COMMON_DEFS_H
#define XRAY_COMMON_DEFS_H

#include <linux/types.h>

#define XRAY_MAX_COMM_LEN 16
#define XRAY_MAX_PATH_LEN 256
#define XRAY_MAX_ARGS_LEN 1024
#define XRAY_MAX_EVENTS 1024

struct process_event {
    __u32 type;
    __u32 pid;
    __u32 ppid;
    __u32 tid;
    __u32 uid;
    __u32 gid;
    __u64 timestamp;
    char comm[XRAY_MAX_COMM_LEN];
    char exe_path[XRAY_MAX_PATH_LEN];
};

struct file_event {
    __u32 type;
    __u32 pid;
    __u64 timestamp;
    __u64 size;
    int flags;
    int mode;
    char path[XRAY_MAX_PATH_LEN];
    char comm[XRAY_MAX_COMM_LEN];
};

struct network_event {
    __u32 type;
    __u32 pid;
    __u64 timestamp;
    __u32 protocol;  // 1=TCP, 2=UDP
    __u16 local_port;
    __u16 remote_port;
    __u32 local_addr;
    __u32 remote_addr;
    __u64 bytes_transferred;
    char comm[XRAY_MAX_COMM_LEN];
};

struct memory_event {
    __u32 type;
    __u32 pid;
    __u64 timestamp;
    __u64 address;
    __u64 size;
    int prot;
    int flags;
    char comm[XRAY_MAX_COMM_LEN];
};

#endif // XRAY_COMMON_DEFS_H
