#include "kernel/ebpf_loader.hpp"
#include <spdlog/spdlog.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <sys/resource.h>
#include <cstring>

namespace xray::kernel {

EbpfLoader::EbpfLoader() {
    // Bump RLIMIT_MEMLOCK for libbpf
    struct rlimit rlim_new = {
        .rlim_cur = RLIM_INFINITY,
        .rlim_max = RLIM_INFINITY,
    };
    setrlimit(RLIMIT_MEMLOCK, &rlim_new);

    libbpf_set_print([](enum libbpf_print_level level, const char* format, va_list args) -> int {
        if (level == LIBBPF_DEBUG) return 0;
        return vfprintf(stderr, format, args);
    });
}

EbpfLoader::~EbpfLoader() {
    stopPolling();
    if (m_rb) {
        ring_buffer__free(m_rb);
    }
    if (m_obj) {
        bpf_object__close(m_obj);
    }
}

bool EbpfLoader::loadProgram(const std::string& objectPath, const std::string& programName) {
    m_obj = bpf_object__open_file(objectPath.c_str(), nullptr);
    if (!m_obj) {
        m_lastError = "Failed to open BPF object: " + objectPath;
        spdlog::error(m_lastError);
        return false;
    }

    int err = bpf_object__load(m_obj);
    if (err) {
        m_lastError = "Failed to load BPF object: " + std::string(strerror(-err));
        spdlog::error(m_lastError);
        bpf_object__close(m_obj);
        m_obj = nullptr;
        return false;
    }

    spdlog::info("Loaded BPF object: {}", objectPath);
    return true;
}

bool EbpfLoader::attachTracepoint(const std::string& category, const std::string& name) {
    if (!m_obj) return false;

    struct bpf_program* prog = bpf_object__find_program_by_name(m_obj, name.c_str());
    if (!prog) {
        m_lastError = "BPF program not found: " + name;
        spdlog::error(m_lastError);
        return false;
    }

    struct bpf_link* link = bpf_program__attach_tracepoint(prog, category.c_str(), name.c_str());
    if (!link) {
        m_lastError = "Failed to attach tracepoint: " + std::string(strerror(errno));
        spdlog::error(m_lastError);
        return false;
    }

    spdlog::info("Attached tracepoint: {}/{}", category, name);
    return true;
}

bool EbpfLoader::attachKprobe(const std::string& functionName) {
    if (!m_obj) return false;

    struct bpf_program* prog = bpf_object__find_program_by_name(m_obj, functionName.c_str());
    if (!prog) {
        m_lastError = "BPF program not found: " + functionName;
        return false;
    }

    struct bpf_link* link = bpf_program__attach_kprobe(prog, false, functionName.c_str());
    if (!link) {
        m_lastError = "Failed to attach kprobe: " + std::string(strerror(errno));
        return false;
    }

    return true;
}

void EbpfLoader::setEventBus(std::shared_ptr<engine::EventBus> bus) {
    m_eventBus = std::move(bus);
}

void EbpfLoader::startPolling() {
    if (!m_obj || m_running.exchange(true)) return;

    // Find ring buffer map
    struct bpf_map* rb_map = bpf_object__find_map_by_name(m_obj, "process_events");
    if (!rb_map) {
        rb_map = bpf_object__find_map_by_name(m_obj, "rb");
    }
    if (!rb_map) {
        rb_map = bpf_object__find_map_by_name(m_obj, "ringbuf");
    }

    if (!rb_map) {
        spdlog::warn("No ring buffer map found in BPF object");
        m_running = false;
        return;
    }

    m_rb = ring_buffer__new(bpf_map__fd(rb_map), handleRingBufferEvent, this, nullptr);
    if (!m_rb) {
        spdlog::error("Failed to create ring buffer");
        m_running = false;
        return;
    }

    m_pollThread = std::thread(&EbpfLoader::pollLoop, this);
    spdlog::info("eBPF polling started");
}

void EbpfLoader::stopPolling() {
    if (!m_running.exchange(false)) return;

    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }

    spdlog::info("eBPF polling stopped");
}

void EbpfLoader::pollLoop() {
    while (m_running) {
        int err = ring_buffer__poll(m_rb, 100);
        if (err == -EINTR) break;
        if (err < 0) {
            spdlog::error("Ring buffer poll error: {}", err);
            break;
        }
    }
}

auto EbpfLoader::handleRingBufferEvent(void* ctx, void* data, size_t dataSize) -> int {
    auto* loader = static_cast<EbpfLoader*>(ctx);
    if (!loader->m_eventBus) return 0;

    // Parse the raw event data and convert to engine::Event
    // The data structure depends on the specific eBPF program
    // For process events:
    struct RawProcessEvent {
        uint32_t type;
        uint32_t pid;
        uint32_t ppid;
        uint32_t tid;
        uint32_t uid;
        uint32_t gid;
        uint64_t timestamp;
        char comm[16];
        char exe_path[256];
    };

    if (dataSize >= sizeof(RawProcessEvent)) {
        auto* raw = static_cast<RawProcessEvent*>(data);

        engine::ProcessInfo info;
        info.pid = raw->pid;
        info.ppid = raw->ppid;
        info.tid = raw->tid;
        info.uid = raw->uid;
        info.gid = raw->gid;
        info.comm = std::string(raw->comm, strnlen(raw->comm, 16));
        info.exePath = std::string(raw->exe_path, strnlen(raw->exe_path, 256));

        engine::EventType eventType = engine::EventType::Unknown;
        switch (raw->type) {
            case 1: eventType = engine::EventType::ProcessExec; break;
            case 2: eventType = engine::EventType::ProcessFork; break;
            case 3: eventType = engine::EventType::ProcessExit; break;
        }

        engine::Event event(eventType, info);
        event.setSource("ebpf");
        loader->m_eventBus->publish(std::move(event));
    }

    return 0;
}

} // namespace xray::kernel
