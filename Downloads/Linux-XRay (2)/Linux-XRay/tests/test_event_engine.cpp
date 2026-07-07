#include <gtest/gtest.h>
#include "engine/event.hpp"
#include "engine/event_bus.hpp"
#include "engine/event_dispatcher.hpp"
#include "engine/event_filter.hpp"
#include "engine/timestamp_engine.hpp"
#include "engine/serializer.hpp"
#include "engine/cache.hpp"

using namespace xray::engine;

class EventEngineTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(EventEngineTest, EventCreation) {
    Event event(EventType::ProcessExec);
    EXPECT_EQ(event.getType(), EventType::ProcessExec);
    EXPECT_EQ(event.getSeverity(), EventSeverity::Info);
    EXPECT_GT(event.getId(), 0);
}

TEST_F(EventEngineTest, EventBusPublishSubscribe) {
    auto bus = std::make_shared<EventBus>();
    bus->start();

    bool received = false;
    bus->subscribe(EventType::ProcessExec, [&received](const Event&) {
        received = true;
    });

    bus->publish(Event(EventType::ProcessExec));
    bus->waitUntilEmpty();

    EXPECT_TRUE(received);
    bus->stop();
}

TEST_F(EventEngineTest, EventFilterByType) {
    auto filter = EventFilter::byType(EventType::ProcessExec);

    Event execEvent(EventType::ProcessExec);
    Event forkEvent(EventType::ProcessFork);

    EXPECT_TRUE(filter(execEvent));
    EXPECT_FALSE(filter(forkEvent));
}

TEST_F(EventEngineTest, EventFilterComposite) {
    auto filter1 = EventFilter::byType(EventType::ProcessExec);
    auto filter2 = EventFilter::bySeverity(EventSeverity::Warning);

    auto composite = EventFilter::compositeAnd({filter1, filter2});

    Event event(EventType::ProcessExec);
    event.setSeverity(EventSeverity::Warning);

    EXPECT_TRUE(composite(event));

    event.setSeverity(EventSeverity::Info);
    EXPECT_FALSE(composite(event));
}

TEST_F(EventEngineTest, TimestampEngine) {
    TimestampEngine engine;

    Event event(EventType::ProcessExec);
    engine.recordEvent(event);

    EXPECT_EQ(engine.getEventCount(), 1);

    auto now = engine.now();
    auto events = engine.getEventsBefore(now + std::chrono::nanoseconds(1));
    EXPECT_EQ(events.size(), 1);
}

TEST_F(EventEngineTest, SerializerJson) {
    Serializer serializer;
    Event event(EventType::ProcessExec);

    auto data = serializer.serialize(event, SerializationFormat::Json);
    EXPECT_FALSE(data.empty());
}

TEST_F(EventEngineTest, CacheLRU) {
    LRUCache<int, std::string> cache(2);
    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");

    EXPECT_FALSE(cache.get(1).has_value());
    EXPECT_TRUE(cache.get(2).has_value());
    EXPECT_TRUE(cache.get(3).has_value());
}

TEST_F(EventEngineTest, EventDispatcherFilter) {
    auto bus = std::make_shared<EventBus>();
    auto dispatcher = std::make_shared<EventDispatcher>(bus);

    bool received = false;
    bus->subscribe(EventType::ProcessExec, [&received](const Event&) {
        received = true;
    });
    bus->start();

    dispatcher->registerFilter([](const Event& e) {
        return e.getType() == EventType::ProcessExec;
    });

    dispatcher->dispatch(Event(EventType::ProcessExec));
    bus->waitUntilEmpty();

    EXPECT_TRUE(received);
    bus->stop();
}
