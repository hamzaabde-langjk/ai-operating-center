#pragma once

#include "security/security_analyzer.hpp"

namespace xray::security {

class ReverseShellDetector : public ThreatDetector {
public:
    auto detect(const engine::Event& event) -> std::optional<ThreatAlert> override;
    auto getName() const -> std::string override { return "ReverseShellDetector"; }
};

class CryptominerDetector : public ThreatDetector {
public:
    auto detect(const engine::Event& event) -> std::optional<ThreatAlert> override;
    auto getName() const -> std::string override { return "CryptominerDetector"; }
};

class DataExfiltrationDetector : public ThreatDetector {
public:
    auto detect(const engine::Event& event) -> std::optional<ThreatAlert> override;
    auto getName() const -> std::string override { return "DataExfiltrationDetector"; }
};

} // namespace xray::security
