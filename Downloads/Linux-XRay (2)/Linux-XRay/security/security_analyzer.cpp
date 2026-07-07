#include "security/security_analyzer.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <regex>

namespace xray::security {

SecurityAnalyzer::SecurityAnalyzer() = default;

void SecurityAnalyzer::analyzeEvent(const engine::Event& event) {
    detectPrivilegeEscalation(event);
    detectSuspiciousProcess(event);
    detectCodeInjection(event);
    detectPersistence(event);
    detectFileAnomaly(event);
    detectNetworkAnomaly(event);
    detectContainerEscape(event);

    // Check custom rules
    for (const auto& rule : m_customRules) {
        try {
            if (rule.detector(event)) {
                addAlert(rule.type, rule.level, rule.name,
                        "Custom rule triggered: " + rule.name,
                        event.getId(), {});
            }
        } catch (const std::exception& e) {
            spdlog::error("Custom rule '{}' error: {}", rule.name, e.what());
        }
    }
}

void SecurityAnalyzer::analyzeGraph(const graph::Graph& graph) {
    // Analyze graph structure for anomalies
    // e.g., isolated nodes, unusual connection patterns
    auto components = graph.getConnectedComponents();
    if (components.size() > 100) {  // Arbitrary threshold
        addAlert(ThreatType::NetworkAnomaly, ThreatLevel::Low,
                "Unusual graph fragmentation",
                "Graph has " + std::to_string(components.size()) + " disconnected components",
                0, {{"component_count", std::to_string(components.size()), 0.6}});
    }
}

auto SecurityAnalyzer::getAlerts() const -> const std::vector<ThreatAlert>& {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_alerts;
}

auto SecurityAnalyzer::getAlertsByLevel(ThreatLevel minLevel) const -> std::vector<ThreatAlert> {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<ThreatAlert> result;
    std::copy_if(m_alerts.begin(), m_alerts.end(), std::back_inserter(result),
                 [minLevel](const auto& alert) { return alert.level >= minLevel; });
    return result;
}

auto SecurityAnalyzer::getAlertsByType(ThreatType type) const -> std::vector<ThreatAlert> {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<ThreatAlert> result;
    std::copy_if(m_alerts.begin(), m_alerts.end(), std::back_inserter(result),
                 [type](const auto& alert) { return alert.type == type; });
    return result;
}

auto SecurityAnalyzer::getUnacknowledgedAlerts() const -> std::vector<ThreatAlert> {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<ThreatAlert> result;
    std::copy_if(m_alerts.begin(), m_alerts.end(), std::back_inserter(result),
                 [](const auto& alert) { return !alert.acknowledged; });
    return result;
}

void SecurityAnalyzer::acknowledgeAlert(uint64_t alertId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& alert : m_alerts) {
        if (alert.id == alertId) {
            alert.acknowledged = true;
            spdlog::info("Alert {} acknowledged", alertId);
            break;
        }
    }
}

void SecurityAnalyzer::clearAlerts() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_alerts.clear();
}

void SecurityAnalyzer::registerCustomRule(std::string name,
                                         std::function<bool(const engine::Event&)> detector,
                                         ThreatType type, ThreatLevel level) {
    m_customRules.push_back({std::move(name), std::move(detector), type, level});
    spdlog::info("Registered custom security rule: {}", m_customRules.back().name);
}

void SecurityAnalyzer::setBaseline(const std::unordered_map<std::string, double>& baseline) {
    m_baseline = baseline;
}

auto SecurityAnalyzer::detectAnomaly(const std::string& metric, double value) const -> bool {
    auto it = m_baseline.find(metric);
    if (it == m_baseline.end()) return false;

    auto& history = m_metricHistory[metric];
    history.push_back(value);
    if (history.size() > 100) history.erase(history.begin());

    double baseline = it->second;
    double deviation = std::abs(value - baseline) / baseline;
    return deviation > 2.0;  // 200% deviation threshold
}

void SecurityAnalyzer::detectPrivilegeEscalation(const engine::Event& event) {
    if (event.getType() != engine::EventType::ProcessExec) return;

    // Check for setuid/setgid execution
    // In a real implementation, we'd check the event data for file mode
    // This is a simplified placeholder
}

void SecurityAnalyzer::detectSuspiciousProcess(const engine::Event& event) {
    if (event.getType() != engine::EventType::ProcessExec) return;

    // Check for known suspicious process names
    // Would extract process name from event data in full implementation
}

void SecurityAnalyzer::detectCodeInjection(const engine::Event& event) {
    // Monitor for ptrace, process_vm_writev, etc.
    // Would need specific eBPF probes for these syscalls
}

void SecurityAnalyzer::detectPersistence(const engine::Event& event) {
    if (event.getType() != engine::EventType::FileWrite) return;

    // Check for writes to common persistence locations
    // e.g., /etc/cron.d, ~/.bashrc, systemd service files
}

void SecurityAnalyzer::detectFileAnomaly(const engine::Event& event) {
    if (event.getType() != engine::EventType::FileOpen && 
        event.getType() != engine::EventType::FileWrite) return;

    // Check for access to sensitive files
    // /etc/shadow, /etc/passwd modifications, etc.
}

void SecurityAnalyzer::detectNetworkAnomaly(const engine::Event& event) {
    if (event.getType() != engine::EventType::SocketConnect &&
        event.getType() != engine::EventType::SocketSend) return;

    // Check for connections to suspicious IPs/ports
    // Unusual outbound connections
}

void SecurityAnalyzer::detectContainerEscape(const engine::Event& event) {
    // Monitor for attempts to access host resources from containers
    // e.g., /proc/1/root, cgroup manipulation
}

void SecurityAnalyzer::addAlert(ThreatType type, ThreatLevel level, std::string title,
                               std::string description, uint64_t eventId,
                               std::vector<ThreatIndicator> indicators) {
    std::lock_guard<std::mutex> lock(m_mutex);

    ThreatAlert alert;
    alert.id = m_nextAlertId++;
    alert.type = type;
    alert.level = level;
    alert.title = std::move(title);
    alert.description = std::move(description);
    alert.timestamp = std::chrono::system_clock::now();
    alert.relatedEventId = eventId;
    alert.indicators = std::move(indicators);

    // Add recommended actions based on threat type
    switch (type) {
        case ThreatType::PrivilegeEscalation:
            alert.recommendedActions = {
                "Investigate process tree",
                "Check audit logs",
                "Review sudoers configuration"
            };
            break;
        case ThreatType::SuspiciousProcess:
            alert.recommendedActions = {
                "Capture process memory dump",
                "Analyze process behavior",
                "Check parent process legitimacy"
            };
            break;
        case ThreatType::NetworkAnomaly:
            alert.recommendedActions = {
                "Block suspicious connections",
                "Analyze network traffic",
                "Check DNS logs"
            };
            break;
        default:
            alert.recommendedActions = {"Review event details", "Check system logs"};
    }

    m_alerts.push_back(std::move(alert));

    spdlog::warn("Security Alert [{}]: {} - {}", 
                 static_cast<int>(level), alert.title, alert.description);
}

auto SecurityAnalyzer::isSuspiciousPath(const std::string& path) const -> bool {
    static const std::vector<std::string> suspiciousPaths = {
        "/tmp/.", "/dev/shm/.", "/var/tmp/.", "/run/.",
        "/proc/self/", "/proc/1/", "/sys/"
    };

    for (const auto& suspicious : suspiciousPaths) {
        if (path.find(suspicious) != std::string::npos) {
            return true;
        }
    }
    return false;
}

auto SecurityAnalyzer::isSuspiciousNetworkConnection(const std::string& remoteAddr,
                                                      uint16_t port) const -> bool {
    // Check for known C2 ports, Tor exit nodes, etc.
    static const std::unordered_set<uint16_t> suspiciousPorts = {
        4444, 5555, 6666, 7777, 8888, 9999,  // Common backdoor ports
        31337, 12345, 54321                   // Classic backdoor ports
    };
    return suspiciousPorts.count(port) > 0;
}

} // namespace xray::security
