#include <benchmark/benchmark.h>
#include "engine/event.hpp"
#include "engine/event_bus.hpp"
#include "graph/graph.hpp"

using namespace xray;

static void BM_EventCreation(benchmark::State& state) {
    for (auto _ : state) {
        engine::Event event(engine::EventType::ProcessExec);
        benchmark::DoNotOptimize(event);
    }
}
BENCHMARK(BM_EventCreation);

static void BM_EventBusPublish(benchmark::State& state) {
    auto bus = std::make_shared<engine::EventBus>();
    bus->start();

    for (auto _ : state) {
        bus->publish(engine::Event(engine::EventType::ProcessExec));
    }

    bus->stop();
}
BENCHMARK(BM_EventBusPublish)->Iterations(10000);

static void BM_GraphAddNode(benchmark::State& state) {
    graph::Graph g;
    for (auto _ : state) {
        auto node = g.addNode(graph::NodeType::Process, "test");
        benchmark::DoNotOptimize(node);
    }
}
BENCHMARK(BM_GraphAddNode);

static void BM_GraphTraverse(benchmark::State& state) {
    graph::Graph g;
    auto root = g.addNode(graph::NodeType::Process, "root");

    for (int i = 0; i < 100; ++i) {
        auto child = g.addNode(graph::NodeType::Process, std::to_string(i));
        g.addEdge(root->getId(), child->getId(), graph::EdgeType::ParentChild);
    }

    for (auto _ : state) {
        g.traverseBFS(root->getId(), [](auto) {});
    }
}
BENCHMARK(BM_GraphTraverse);

BENCHMARK_MAIN();
