#include "engine/serializer.hpp"
#include <spdlog/spdlog.h>
#include <sstream>
#include <iomanip>

namespace xray::engine {

auto Serializer::serialize(const Event& event, SerializationFormat format) 
    -> std::vector<uint8_t> {
    switch (format) {
        case SerializationFormat::Json:
            return serializeJson(event);
        case SerializationFormat::Binary:
            return serializeBinary(event);
        default:
            spdlog::error("Unsupported serialization format");
            return {};
    }
}

auto Serializer::deserialize(const std::vector<uint8_t>& data, SerializationFormat format)
    -> std::optional<Event> {
    switch (format) {
        case SerializationFormat::Json:
            return deserializeJson(data);
        case SerializationFormat::Binary:
            return deserializeBinary(data);
        default:
            spdlog::error("Unsupported deserialization format");
            return std::nullopt;
    }
}

auto Serializer::serializeBatch(const std::vector<Event>& events, 
                              SerializationFormat format) -> std::vector<uint8_t> {
    std::vector<uint8_t> result;
    for (const auto& event : events) {
        auto serialized = serialize(event, format);
        result.insert(result.end(), serialized.begin(), serialized.end());
    }
    return result;
}

auto Serializer::deserializeBatch(const std::vector<uint8_t>& data,
                                SerializationFormat format) -> std::vector<Event> {
    // Simplified - would need proper framing for real implementation
    auto event = deserialize(data, format);
    if (event) {
        return {*event};
    }
    return {};
}

auto Serializer::serializeJson(const Event& event) -> std::vector<uint8_t> {
    std::string json = event.toJson();
    return std::vector<uint8_t>(json.begin(), json.end());
}

auto Serializer::serializeBinary(const Event& event) -> std::vector<uint8_t> {
    // Simplified binary serialization
    std::vector<uint8_t> data;
    uint64_t id = event.getId();
    data.insert(data.end(), reinterpret_cast<uint8_t*>(&id), 
                reinterpret_cast<uint8_t*>(&id) + sizeof(id));
    return data;
}

auto Serializer::deserializeJson(const std::vector<uint8_t>& data) -> std::optional<Event> {
    std::string json(data.begin(), data.end());
    return Event::fromJson(json);
}

auto Serializer::deserializeBinary(const std::vector<uint8_t>& data) -> std::optional<Event> {
    // Simplified - would need proper binary format
    return std::nullopt;
}

} // namespace xray::engine
