#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include "common_defs.h"

char LICENSE[] SEC("license") = "GPL";

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024);
} file_events SEC(".maps");

SEC("tp/syscalls/sys_enter_openat")
int tracepoint__syscalls__sys_enter_openat(struct trace_event_raw_sys_enter *ctx) {
    struct file_event *event = bpf_ringbuf_reserve(&file_events, sizeof(*event), 0);
    if (!event)
        return 0;

    event->type = 10;  // FileOpen
    event->timestamp = bpf_ktime_get_ns();
    event->pid = bpf_get_current_pid_tgid() >> 32;

    struct task_struct *task = (struct task_struct *)bpf_get_current_task();
    bpf_probe_read_kernel_str(event->comm, sizeof(event->comm), task->comm);

    // Read filename from user space
    const char *filename = (const char *)ctx->args[1];
    bpf_probe_read_user_str(event->path, sizeof(event->path), filename);

    event->flags = ctx->args[2];
    event->mode = ctx->args[3];

    bpf_ringbuf_submit(event, 0);
    return 0;
}

SEC("tp/syscalls/sys_enter_read")
int tracepoint__syscalls__sys_enter_read(struct trace_event_raw_sys_enter *ctx) {
    struct file_event *event = bpf_ringbuf_reserve(&file_events, sizeof(*event), 0);
    if (!event)
        return 0;

    event->type = 11;  // FileRead
    event->timestamp = bpf_ktime_get_ns();
    event->pid = bpf_get_current_pid_tgid() >> 32;
    event->size = ctx->args[2];

    struct task_struct *task = (struct task_struct *)bpf_get_current_task();
    bpf_probe_read_kernel_str(event->comm, sizeof(event->comm), task->comm);

    // Try to get file path from fd
    int fd = ctx->args[0];
    struct files_struct *files = BPF_CORE_READ(task, files);
    struct fdtable *fdt = BPF_CORE_READ(files, fdt);
    struct file **farray = BPF_CORE_READ(fdt, fd);
    struct file *file = NULL;
    bpf_probe_read_kernel(&file, sizeof(file), &farray[fd]);

    if (file) {
        struct path fpath = BPF_CORE_READ(file, f_path);
        struct dentry *dentry = fpath.dentry;
        bpf_probe_read_kernel_str(event->path, sizeof(event->path), 
                                 BPF_CORE_READ(dentry, d_name.name));
    } else {
        __builtin_memcpy(event->path, "unknown", 8);
    }

    bpf_ringbuf_submit(event, 0);
    return 0;
}

SEC("tp/syscalls/sys_enter_write")
int tracepoint__syscalls__sys_enter_write(struct trace_event_raw_sys_enter *ctx) {
    struct file_event *event = bpf_ringbuf_reserve(&file_events, sizeof(*event), 0);
    if (!event)
        return 0;

    event->type = 12;  // FileWrite
    event->timestamp = bpf_ktime_get_ns();
    event->pid = bpf_get_current_pid_tgid() >> 32;
    event->size = ctx->args[2];

    struct task_struct *task = (struct task_struct *)bpf_get_current_task();
    bpf_probe_read_kernel_str(event->comm, sizeof(event->comm), task->comm);

    int fd = ctx->args[0];
    struct files_struct *files = BPF_CORE_READ(task, files);
    struct fdtable *fdt = BPF_CORE_READ(files, fdt);
    struct file **farray = BPF_CORE_READ(fdt, fd);
    struct file *file = NULL;
    bpf_probe_read_kernel(&file, sizeof(file), &farray[fd]);

    if (file) {
        struct path fpath = BPF_CORE_READ(file, f_path);
        struct dentry *dentry = fpath.dentry;
        bpf_probe_read_kernel_str(event->path, sizeof(event->path), 
                                 BPF_CORE_READ(dentry, d_name.name));
    } else {
        __builtin_memcpy(event->path, "unknown", 8);
    }

    bpf_ringbuf_submit(event, 0);
    return 0;
}

SEC("tp/syscalls/sys_enter_unlinkat")
int tracepoint__syscalls__sys_enter_unlinkat(struct trace_event_raw_sys_enter *ctx) {
    struct file_event *event = bpf_ringbuf_reserve(&file_events, sizeof(*event), 0);
    if (!event)
        return 0;

    event->type = 15;  // FileUnlink
    event->timestamp = bpf_ktime_get_ns();
    event->pid = bpf_get_current_pid_tgid() >> 32;

    struct task_struct *task = (struct task_struct *)bpf_get_current_task();
    bpf_probe_read_kernel_str(event->comm, sizeof(event->comm), task->comm);

    const char *filename = (const char *)ctx->args[1];
    bpf_probe_read_user_str(event->path, sizeof(event->path), filename);

    bpf_ringbuf_submit(event, 0);
    return 0;
}
