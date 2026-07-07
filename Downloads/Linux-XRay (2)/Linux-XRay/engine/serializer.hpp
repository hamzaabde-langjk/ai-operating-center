#pragma once

#include "engine/event.hpp"
#include <string>
#include <vector>
#include <optional>

namespace xray::engine {

enum class SerializationFormat {
    Json,
    Binary,
    Csv,
    Compressed
};

class Serializer {
public:
    [[nodiscard]] auto serialize(const Event& event, SerializationFormat format) 
        -> std::vector<uint8_t>;

    [[nodiscard]] auto deserialize(const std::vector<uint8_t>& data, SerializationFormat format) 
        -> std::optional<Event>;

    [[nodiscard]] auto serializeBatch(const std::vector<Event>& events, 
                                    SerializationFormat format) -> std::vector<uint8_t>;

    [[nodiscard]] auto deserializeBatch(const std::vector<uint8_t>& data, 
                                      SerializationFormat format) -> std::vector<Event>;

private:
    [[nodiscard]] auto serializeJson(const Event& event) -> std::vector<uint8_t>;
    [[nodiscard]] auto serializeBinary(const Event& event) -> std::vector<uint8_t>;
    [[nodiscard]] auto deserializeJson(const std::vector<uint8_t>& data) -> std::optional<Event>;
    [[nodiscard]] auto deserializeBinary(const std::vector<uint8_t>& data) -> std::optional<Event>;
};

} // namespace xray::engine
