#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include "common_defs.h"

char LICENSE[] SEC("license") = "GPL";

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024);
} network_events SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 4096);
    __type(key, __u64);  // pid_tgid
    __type(value, struct sock *);
} socket_map SEC(".maps");

static __always_inline void read_sock_addr(struct sock *sk, struct network_event *event) {
    struct inet_sock *inet = (struct inet_sock *)sk;

    event->local_addr = BPF_CORE_READ(inet, inet_saddr);
    event->remote_addr = BPF_CORE_READ(inet, inet_daddr);
    event->local_port = bpf_ntohs(BPF_CORE_READ(inet, inet_sport));
    event->remote_port = bpf_ntohs(BPF_CORE_READ(inet, inet_dport));
}

SEC("kprobe/tcp_connect")
int BPF_KPROBE(tcp_connect, struct sock *sk) {
    struct network_event *event = bpf_ringbuf_reserve(&network_events, sizeof(*event), 0);
    if (!event)
        return 0;

    event->type = 20;  // SocketConnect
    event->timestamp = bpf_ktime_get_ns();
    event->pid = bpf_get_current_pid_tgid() >> 32;
    event->protocol = 1;  // TCP

    read_sock_addr(sk, event);

    struct task_struct *task = (struct task_struct *)bpf_get_current_task();
    bpf_probe_read_kernel_str(event->comm, sizeof(event->comm), task->comm);

    bpf_ringbuf_submit(event, 0);
    return 0;
}

SEC("kprobe/tcp_close")
int BPF_KPROBE(tcp_close, struct sock *sk) {
    struct network_event *event = bpf_ringbuf_reserve(&network_events, sizeof(*event), 0);
    if (!event)
        return 0;

    event->type = 21;  // SocketClose (mapped to custom type)
    event->timestamp = bpf_ktime_get_ns();
    event->pid = bpf_get_current_pid_tgid() >> 32;
    event->protocol = 1;

    read_sock_addr(sk, event);

    struct task_struct *task = (struct task_struct *)bpf_get_current_task();
    bpf_probe_read_kernel_str(event->comm, sizeof(event->comm), task->comm);

    bpf_ringbuf_submit(event, 0);
    return 0;
}

SEC("kprobe/udp_sendmsg")
int BPF_KPROBE(udp_sendmsg, struct sock *sk, struct msghdr *msg, size_t size) {
    struct network_event *event = bpf_ringbuf_reserve(&network_events, sizeof(*event), 0);
    if (!event)
        return 0;

    event->type = 24;  // SocketSend
    event->timestamp = bpf_ktime_get_ns();
    event->pid = bpf_get_current_pid_tgid() >> 32;
    event->protocol = 2;  // UDP
    event->bytes_transferred = size;

    read_sock_addr(sk, event);

    struct task_struct *task = (struct task_struct *)bpf_get_current_task();
    bpf_probe_read_kernel_str(event->comm, sizeof(event->comm), task->comm);

    bpf_ringbuf_submit(event, 0);
    return 0;
}

SEC("kprobe/udp_recvmsg")
int BPF_KPROBE(udp_recvmsg, struct sock *sk, struct msghdr *msg, size_t len, int noblock,
               int flags, int *addr_len) {
    struct network_event *event = bpf_ringbuf_reserve(&network_events, sizeof(*event), 0);
    if (!event)
        return 0;

    event->type = 25;  // SocketReceive
    event->timestamp = bpf_ktime_get_ns();
    event->pid = bpf_get_current_pid_tgid() >> 32;
    event->protocol = 2;
    event->bytes_transferred = len;

    read_sock_addr(sk, event);

    struct task_struct *task = (struct task_struct *)bpf_get_current_task();
    bpf_probe_read_kernel_str(event->comm, sizeof(event->comm), task->comm);

    bpf_ringbuf_submit(event, 0);
    return 0;
}
