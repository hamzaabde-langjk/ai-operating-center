#pragma once

#include "engine/event.hpp"
#include "graph/graph.hpp"
#include <vector>
#include <string>
#include <chrono>
#include <unordered_map>
#include <functional>

namespace xray::security {

enum class ThreatLevel : uint8_t {
    None = 0,
    Low,
    Medium,
    High,
    Critical
};

enum class ThreatType : uint32_t {
    Unknown = 0,
    PrivilegeEscalation,
    SuspiciousProcess,
    CodeInjection,
    PersistenceMechanism,
    UnexpectedFileChange,
    NetworkAnomaly,
    ContainerEscape,
    ReverseShell,
    Cryptominer,
    Rootkit,
    DataExfiltration,
    LateralMovement,
    CredentialAccess
};

struct ThreatIndicator {
    std::string name;
    std::string value;
    double confidence;
};

struct ThreatAlert {
    uint64_t id;
    ThreatType type;
    ThreatLevel level;
    std::string title;
    std::string description;
    std::chrono::system_clock::time_point timestamp;
    uint64_t relatedEventId;
    std::vector<ThreatIndicator> indicators;
    std::vector<std::string> recommendedActions;
    bool acknowledged{false};
};

class SecurityAnalyzer {
public:
    SecurityAnalyzer();
    ~SecurityAnalyzer() = default;

    void analyzeEvent(const engine::Event& event);
    void analyzeGraph(const graph::Graph& graph);

    [[nodiscard]] auto getAlerts() const -> const std::vector<ThreatAlert>&;
    [[nodiscard]] auto getAlertsByLevel(ThreatLevel minLevel) const -> std::vector<ThreatAlert>;
    [[nodiscard]] auto getAlertsByType(ThreatType type) const -> std::vector<ThreatAlert>;
    [[nodiscard]] auto getUnacknowledgedAlerts() const -> std::vector<ThreatAlert>;

    void acknowledgeAlert(uint64_t alertId);
    void clearAlerts();

    void registerCustomRule(std::string name, 
                           std::function<bool(const engine::Event&)> detector,
                           ThreatType type, ThreatLevel level);

    void setBaseline(const std::unordered_map<std::string, double>& baseline);
    [[nodiscard]] auto detectAnomaly(const std::string& metric, double value) const -> bool;

private:
    std::vector<ThreatAlert> m_alerts;
    mutable std::mutex m_mutex;
    std::atomic<uint64_t> m_nextAlertId{1};

    struct CustomRule {
        std::string name;
        std::function<bool(const engine::Event&)> detector;
        ThreatType type;
        ThreatLevel level;
    };
    std::vector<CustomRule> m_customRules;

    std::unordered_map<std::string, double> m_baseline;
    std::unordered_map<std::string, std::vector<double>> m_metricHistory;

    void detectPrivilegeEscalation(const engine::Event& event);
    void detectSuspiciousProcess(const engine::Event& event);
    void detectCodeInjection(const engine::Event& event);
    void detectPersistence(const engine::Event& event);
    void detectFileAnomaly(const engine::Event& event);
    void detectNetworkAnomaly(const engine::Event& event);
    void detectContainerEscape(const engine::Event& event);

    void addAlert(ThreatType type, ThreatLevel level, std::string title,
                  std::string description, uint64_t eventId,
                  std::vector<ThreatIndicator> indicators);

    [[nodiscard]] auto isSuspiciousPath(const std::string& path) const -> bool;
    [[nodiscard]] auto isSuspiciousNetworkConnection(const std::string& remoteAddr, 
                                                      uint16_t port) const -> bool;
};

class ThreatDetector {
public:
    virtual ~ThreatDetector() = default;
    virtual auto detect(const engine::Event& event) -> std::optional<ThreatAlert> = 0;
    virtual auto getName() const -> std::string = 0;
};

} // namespace xray::security
