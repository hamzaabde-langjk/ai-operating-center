#include <gtest/gtest.h>
#include "graph/graph.hpp"
#include "graph/node.hpp"
#include "graph/edge.hpp"

using namespace xray::graph;

class GraphEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        graph = std::make_unique<Graph>();
    }

    std::unique_ptr<Graph> graph;
};

TEST_F(GraphEngineTest, NodeCreation) {
    auto node = graph->addNode(NodeType::Process, "test_process");
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->getType(), NodeType::Process);
    EXPECT_EQ(node->getName(), "test_process");
    EXPECT_EQ(graph->getNodeCount(), 1);
}

TEST_F(GraphEngineTest, EdgeCreation) {
    auto node1 = graph->addNode(NodeType::Process, "parent");
    auto node2 = graph->addNode(NodeType::Process, "child");

    auto edge = graph->addEdge(node1->getId(), node2->getId(), EdgeType::ParentChild);
    ASSERT_NE(edge, nullptr);
    EXPECT_EQ(edge->getSourceId(), node1->getId());
    EXPECT_EQ(edge->getTargetId(), node2->getId());
    EXPECT_EQ(graph->getEdgeCount(), 1);
}

TEST_F(GraphEngineTest, NodeRetrieval) {
    auto node = graph->addNode(NodeType::File, "test.txt");
    auto retrieved = graph->getNode(node->getId());

    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->getName(), "test.txt");
}

TEST_F(GraphEngineTest, NodeRemoval) {
    auto node = graph->addNode(NodeType::Process, "temp");
    auto id = node->getId();

    graph->removeNode(id);
    EXPECT_EQ(graph->getNodeCount(), 0);
    EXPECT_EQ(graph->getNode(id), nullptr);
}

TEST_F(GraphEngineTest, EdgeRemoval) {
    auto n1 = graph->addNode(NodeType::Process, "a");
    auto n2 = graph->addNode(NodeType::Process, "b");
    auto edge = graph->addEdge(n1->getId(), n2->getId(), EdgeType::Dependency);
    auto edgeId = edge->getId();

    graph->removeEdge(edgeId);
    EXPECT_EQ(graph->getEdgeCount(), 0);
}

TEST_F(GraphEngineTest, GetNodesByType) {
    graph->addNode(NodeType::Process, "p1");
    graph->addNode(NodeType::Process, "p2");
    graph->addNode(NodeType::File, "f1");

    auto processes = graph->getNodesByType(NodeType::Process);
    EXPECT_EQ(processes.size(), 2);

    auto files = graph->getNodesByType(NodeType::File);
    EXPECT_EQ(files.size(), 1);
}

TEST_F(GraphEngineTest, GraphTraversal) {
    auto n1 = graph->addNode(NodeType::Process, "root");
    auto n2 = graph->addNode(NodeType::Process, "child1");
    auto n3 = graph->addNode(NodeType::Process, "child2");
    auto n4 = graph->addNode(NodeType::Process, "grandchild");

    graph->addEdge(n1->getId(), n2->getId(), EdgeType::ParentChild);
    graph->addEdge(n1->getId(), n3->getId(), EdgeType::ParentChild);
    graph->addEdge(n2->getId(), n4->getId(), EdgeType::ParentChild);

    std::vector<uint64_t> visited;
    graph->traverseBFS(n1->getId(), [&visited](auto node) {
        visited.push_back(node->getId());
    });

    EXPECT_EQ(visited.size(), 4);
    EXPECT_EQ(visited[0], n1->getId());
}

TEST_F(GraphEngineTest, PathFinding) {
    auto n1 = graph->addNode(NodeType::Process, "a");
    auto n2 = graph->addNode(NodeType::Process, "b");
    auto n3 = graph->addNode(NodeType::Process, "c");

    graph->addEdge(n1->getId(), n2->getId(), EdgeType::Dependency);
    graph->addEdge(n2->getId(), n3->getId(), EdgeType::Dependency);

    auto path = graph->findPath(n1->getId(), n3->getId());
    EXPECT_EQ(path.size(), 3);
    EXPECT_EQ(path[0], n1->getId());
    EXPECT_EQ(path[1], n2->getId());
    EXPECT_EQ(path[2], n3->getId());
}

TEST_F(GraphEngineTest, ConnectedComponents) {
    auto n1 = graph->addNode(NodeType::Process, "a");
    auto n2 = graph->addNode(NodeType::Process, "b");
    auto n3 = graph->addNode(NodeType::Process, "c");
    auto n4 = graph->addNode(NodeType::Process, "d");

    graph->addEdge(n1->getId(), n2->getId(), EdgeType::Dependency);

    auto components = graph->getConnectedComponents();
    EXPECT_EQ(components.size(), 3);
}

TEST_F(GraphEngineTest, NodeMetadata) {
    auto node = graph->addNode(NodeType::Process, "test");
    node->setMetadata("pid", "1234");
    node->setMetadata("user", "root");

    EXPECT_EQ(node->getMetadata("pid"), "1234");
    EXPECT_EQ(node->getMetadata("user"), "root");
    EXPECT_FALSE(node->getMetadata("nonexistent").has_value());

    auto allMeta = node->getAllMetadata();
    EXPECT_EQ(allMeta.size(), 2);
}
