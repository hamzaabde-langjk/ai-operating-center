#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include "common_defs.h"

char LICENSE[] SEC("license") = "GPL";

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024);  // 256KB ring buffer
} process_events SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 8192);
    __type(key, __u32);
    __type(value, __u32);
} process_parents SEC(".maps");

static __always_inline void get_task_info(struct task_struct *task, struct process_event *event) {
    event->pid = BPF_CORE_READ(task, tgid);
    event->tid = BPF_CORE_READ(task, pid);
    event->uid = BPF_CORE_READ(task, cred, uid.val);
    event->gid = BPF_CORE_READ(task, cred, gid.val);

    bpf_probe_read_kernel_str(event->comm, sizeof(event->comm), task->comm);
}

SEC("tp/sched/sched_process_exec")
int tracepoint__sched__sched_process_exec(struct trace_event_raw_sched_process_exec *ctx) {
    struct process_event *event = bpf_ringbuf_reserve(&process_events, sizeof(*event), 0);
    if (!event)
        return 0;

    event->type = 1;  // ProcessExec
    event->timestamp = bpf_ktime_get_ns();

    struct task_struct *task = (struct task_struct *)bpf_get_current_task();
    get_task_info(task, event);

    // Get parent PID
    struct task_struct *parent = BPF_CORE_READ(task, real_parent);
    event->ppid = BPF_CORE_READ(parent, tgid);

    // Store parent relationship
    bpf_map_update_elem(&process_parents, &event->pid, &event->ppid, BPF_ANY);

    // Get executable path from mm_struct
    struct mm_struct *mm = BPF_CORE_READ(task, mm);
    if (mm) {
        struct file *exe_file = BPF_CORE_READ(mm, exe_file);
        if (exe_file) {
            struct path fpath = BPF_CORE_READ(exe_file, f_path);
            struct dentry *dentry = fpath.dentry;
            bpf_probe_read_kernel_str(event->exe_path, sizeof(event->exe_path), 
                                     BPF_CORE_READ(dentry, d_name.name));
        }
    }

    bpf_ringbuf_submit(event, 0);
    return 0;
}

SEC("tp/sched/sched_process_fork")
int tracepoint__sched__sched_process_fork(struct trace_event_raw_sched_process_fork *ctx) {
    struct process_event *event = bpf_ringbuf_reserve(&process_events, sizeof(*event), 0);
    if (!event)
        return 0;

    event->type = 2;  // ProcessFork
    event->timestamp = bpf_ktime_get_ns();
    event->pid = ctx->child_pid;
    event->ppid = ctx->parent_pid;
    event->tid = ctx->child_tid;

    struct task_struct *task = (struct task_struct *)bpf_get_current_task();
    event->uid = BPF_CORE_READ(task, cred, uid.val);
    event->gid = BPF_CORE_READ(task, cred, gid.val);

    bpf_probe_read_kernel_str(event->comm, sizeof(event->comm), ctx->child_comm);

    bpf_map_update_elem(&process_parents, &event->pid, &event->ppid, BPF_ANY);

    bpf_ringbuf_submit(event, 0);
    return 0;
}

SEC("tp/sched/sched_process_exit")
int tracepoint__sched__sched_process_exit(struct trace_event_raw_sched_process_template *ctx) {
    struct process_event *event = bpf_ringbuf_reserve(&process_events, sizeof(*event), 0);
    if (!event)
        return 0;

    event->type = 3;  // ProcessExit
    event->timestamp = bpf_ktime_get_ns();

    struct task_struct *task = (struct task_struct *)bpf_get_current_task();
    get_task_info(task, event);

    __u32 *ppid = bpf_map_lookup_elem(&process_parents, &event->pid);
    if (ppid)
        event->ppid = *ppid;
    else
        event->ppid = 0;

    bpf_map_delete_elem(&process_parents, &event->pid);

    bpf_ringbuf_submit(event, 0);
    return 0;
}

SEC("tp/sched/sched_process_free")
int tracepoint__sched__sched_process_free(struct trace_event_raw_sched_process_template *ctx) {
    // Cleanup any remaining resources
    return 0;
}
