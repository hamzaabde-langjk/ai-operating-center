#pragma once

#include "renderer/shader.hpp"
#include "renderer/camera.hpp"
#include "renderer/scene.hpp"
#include "graph/graph_renderer.hpp"
#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>
#include <QTimer>
#include <memory>

namespace xray::renderer {

class GlRenderer : public QOpenGLWidget, public QOpenGLFunctions_4_5_Core, public graph::GraphRenderer {
    Q_OBJECT

public:
    explicit GlRenderer(QWidget* parent = nullptr);
    ~GlRenderer() override;

    // GraphRenderer interface
    void initialize() override;
    void shutdown() override;
    void render(const graph::Graph& graph) override;
    void updateNodePosition(uint64_t nodeId, float x, float y, float z) override;
    void setCamera(float x, float y, float z, float pitch, float yaw) override;
    void onNodeClick(std::function<void(uint64_t)> callback) override;
    void onNodeHover(std::function<void(uint64_t)> callback) override;

    void setScene(std::shared_ptr<Scene> scene);
    [[nodiscard]] auto getCamera() -> Camera& { return m_camera; }

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void setupShaders();
    void setupBuffers();
    void renderNodes();
    void renderEdges();
    void renderParticles();
    void pickNode(int x, int y);

    std::shared_ptr<Scene> m_scene;
    Camera m_camera;
    Shader m_nodeShader;
    Shader m_edgeShader;
    Shader m_particleShader;

    unsigned int m_nodeVAO{0}, m_nodeVBO{0};
    unsigned int m_edgeVAO{0}, m_edgeVBO{0};
    unsigned int m_particleVAO{0}, m_particleVBO{0};

    QTimer* m_updateTimer{nullptr};

    bool m_mousePressed{false};
    QPoint m_lastMousePos;

    std::function<void(uint64_t)> m_nodeClickCallback;
    std::function<void(uint64_t)> m_nodeHoverCallback;

    float m_deltaTime{0.016f};
    std::chrono::steady_clock::time_point m_lastFrameTime;
};

} // namespace xray::renderer
