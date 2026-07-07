#pragma once

#include "engine/event.hpp"
#include "engine/event_bus.hpp"
#include <string>
#include <thread>
#include <atomic>
#include <memory>

// Forward declarations for libbpf types
struct bpf_object;
struct ring_buffer;

namespace xray::kernel {

class EbpfLoader {
public:
    EbpfLoader();
    ~EbpfLoader();

    EbpfLoader(const EbpfLoader&) = delete;
    auto operator=(const EbpfLoader&) -> EbpfLoader& = delete;

    bool loadProgram(const std::string& objectPath, const std::string& programName);
    bool attachTracepoint(const std::string& category, const std::string& name);
    bool attachKprobe(const std::string& functionName);

    void setEventBus(std::shared_ptr<engine::EventBus> bus);
    void startPolling();
    void stopPolling();
    [[nodiscard]] auto isRunning() const -> bool { return m_running; }

    [[nodiscard]] auto getLastError() const -> const std::string& { return m_lastError; }

private:
    void pollLoop();
    static auto handleRingBufferEvent(void* ctx, void* data, size_t dataSize) -> int;

    struct bpf_object* m_obj{nullptr};
    struct ring_buffer* m_rb{nullptr};
    std::shared_ptr<engine::EventBus> m_eventBus;
    std::thread m_pollThread;
    std::atomic<bool> m_running{false};
    std::string m_lastError;
};

} // namespace xray::kernel
