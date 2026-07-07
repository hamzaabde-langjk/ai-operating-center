/* This is a minimal vmlinux.h placeholder.
 * In a real build, generate this with:
 *   bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h
 */

#ifndef __VMLINUX_H__
#define __VMLINUX_H__

#include <linux/types.h>
#include <linux/bpf.h>

/* Minimal definitions for compilation without full BTF */
struct task_struct {
    int pid;
    int tgid;
    struct cred *cred;
    struct mm_struct *mm;
    struct task_struct *real_parent;
    struct files_struct *files;
    char comm[16];
};

struct cred {
    kuid_t uid;
    kgid_t gid;
};

struct mm_struct {
    struct file *exe_file;
};

struct file {
    struct path f_path;
};

struct path {
    struct vfsmount *mnt;
    struct dentry *dentry;
};

struct dentry {
    struct qstr d_name;
};

struct qstr {
    const unsigned char *name;
};

struct vfsmount {};

struct files_struct {
    struct fdtable *fdt;
};

struct fdtable {
    struct file **fd;
};

struct inet_sock {
    __be32 inet_saddr;
    __be32 inet_daddr;
    __be16 inet_sport;
    __be16 inet_dport;
};

struct sock {};

struct msghdr {};

struct trace_event_raw_sched_process_exec {
    u64 __data_loc_filename;
    pid_t pid;
    pid_t old_pid;
};

struct trace_event_raw_sched_process_fork {
    char parent_comm[16];
    pid_t parent_pid;
    char child_comm[16];
    pid_t child_pid;
    pid_t child_tid;
};

struct trace_event_raw_sched_process_template {
    char comm[16];
    pid_t pid;
};

struct trace_event_raw_sys_enter {
    unsigned long long unused;
    long id;
    unsigned long args[6];
};

#endif /* __VMLINUX_H__ */
