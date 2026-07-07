#include "renderer/gl_renderer.hpp"
#include <spdlog/spdlog.h>
#include <QMouseEvent>
#include <QWheelEvent>
#include <glm/gtc/type_ptr.hpp>

namespace xray::renderer {

GlRenderer::GlRenderer(QWidget* parent) : QOpenGLWidget(parent) {
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, QOverload<>::of(&GlRenderer::update));
    m_updateTimer->start(16); // ~60 FPS
}

GlRenderer::~GlRenderer() {
    makeCurrent();
    shutdown();
    doneCurrent();
}

void GlRenderer::initialize() {
    // Called externally to ensure GL context is ready
}

void GlRenderer::shutdown() {
    if (m_nodeVAO) glDeleteVertexArrays(1, &m_nodeVAO);
    if (m_nodeVBO) glDeleteBuffers(1, &m_nodeVBO);
    if (m_edgeVAO) glDeleteVertexArrays(1, &m_edgeVAO);
    if (m_edgeVBO) glDeleteBuffers(1, &m_edgeVBO);
    if (m_particleVAO) glDeleteVertexArrays(1, &m_particleVAO);
    if (m_particleVBO) glDeleteBuffers(1, &m_particleVBO);
}

void GlRenderer::render(const graph::Graph& graph) {
    if (m_scene) {
        m_scene->updateFromGraph(graph);
    }
    update();
}

void GlRenderer::updateNodePosition(uint64_t nodeId, float x, float y, float z) {
    update();
}

void GlRenderer::setCamera(float x, float y, float z, float pitch, float yaw) {
    m_camera.setPosition(glm::vec3(x, y, z));
    m_camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
}

void GlRenderer::onNodeClick(std::function<void(uint64_t)> callback) {
    m_nodeClickCallback = std::move(callback);
}

void GlRenderer::onNodeHover(std::function<void(uint64_t)> callback) {
    m_nodeHoverCallback = std::move(callback);
}

void GlRenderer::setScene(std::shared_ptr<Scene> scene) {
    m_scene = std::move(scene);
}

void GlRenderer::initializeGL() {
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);

    setupShaders();
    setupBuffers();

    spdlog::info("OpenGL renderer initialized");
    const GLubyte* version = glGetString(GL_VERSION);
    if (version) {
        spdlog::info("OpenGL Version: {}", reinterpret_cast<const char*>(version));
    }
}

void GlRenderer::paintGL() {
    auto now = std::chrono::steady_clock::now();
    m_deltaTime = std::chrono::duration<float>(now - m_lastFrameTime).count();
    m_lastFrameTime = now;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!m_scene) return;

    m_scene->updateParticles(m_deltaTime);

    float aspect = static_cast<float>(width()) / static_cast<float>(height());
    glm::mat4 view = m_camera.getViewMatrix();
    glm::mat4 projection = m_camera.getProjectionMatrix(aspect);

    renderEdges();
    renderNodes();
    renderParticles();
}

void GlRenderer::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void GlRenderer::setupShaders() {
    const char* nodeVertex = R"(
        #version 450 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec4 aColor;
        layout(location = 2) in float aSize;

        out vec4 vColor;
        out float vSize;

        uniform mat4 view;
        uniform mat4 projection;

        void main() {
            vColor = aColor;
            vSize = aSize;
            gl_Position = projection * view * vec4(aPos, 1.0);
            gl_PointSize = aSize * 10.0;
        }
    )";

    const char* nodeFragment = R"(
        #version 450 core
        in vec4 vColor;
        in float vSize;
        out vec4 FragColor;

        void main() {
            vec2 coord = gl_PointCoord - vec2(0.5);
            float dist = length(coord);
            if (dist > 0.5) discard;

            float glow = 1.0 - smoothstep(0.2, 0.5, dist);
            FragColor = vec4(vColor.rgb, vColor.a * glow);
        }
    )";

    m_nodeShader.loadFromSource(nodeVertex, nodeFragment);

    const char* edgeVertex = R"(
        #version 450 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec4 aColor;
        out vec4 vColor;
        uniform mat4 view;
        uniform mat4 projection;
        void main() {
            vColor = aColor;
            gl_Position = projection * view * vec4(aPos, 1.0);
        }
    )";

    const char* edgeFragment = R"(
        #version 450 core
        in vec4 vColor;
        out vec4 FragColor;
        void main() {
            FragColor = vColor;
        }
    )";

    m_edgeShader.loadFromSource(edgeVertex, edgeFragment);
}

void GlRenderer::setupBuffers() {
    glGenVertexArrays(1, &m_nodeVAO);
    glGenBuffers(1, &m_nodeVBO);

    glBindVertexArray(m_nodeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_nodeVBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 7));
    glEnableVertexAttribArray(2);

    glGenVertexArrays(1, &m_edgeVAO);
    glGenBuffers(1, &m_edgeVBO);

    glBindVertexArray(m_edgeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_edgeVBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void GlRenderer::renderNodes() {
    if (!m_scene) return;
    const auto& nodes = m_scene->getNodes();
    if (nodes.empty()) return;

    std::vector<float> nodeData;
    nodeData.reserve(nodes.size() * 8);

    for (const auto& node : nodes) {
        nodeData.push_back(node.position.x);
        nodeData.push_back(node.position.y);
        nodeData.push_back(node.position.z);
        nodeData.push_back(node.color.r);
        nodeData.push_back(node.color.g);
        nodeData.push_back(node.color.b);
        nodeData.push_back(node.color.a);
        nodeData.push_back(node.size);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_nodeVBO);
    glBufferData(GL_ARRAY_BUFFER, nodeData.size() * sizeof(float), nodeData.data(), GL_DYNAMIC_DRAW);

    m_nodeShader.use();
    float aspect = static_cast<float>(width()) / static_cast<float>(height());
    m_nodeShader.setMat4("view", m_camera.getViewMatrix());
    m_nodeShader.setMat4("projection", m_camera.getProjectionMatrix(aspect));

    glBindVertexArray(m_nodeVAO);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(nodes.size()));
    glDisable(GL_PROGRAM_POINT_SIZE);
}

void GlRenderer::renderEdges() {
    if (!m_scene) return;
    const auto& edges = m_scene->getEdges();
    if (edges.empty()) return;

    std::vector<float> edgeData;
    edgeData.reserve(edges.size() * 14);

    for (const auto& edge : edges) {
        edgeData.push_back(edge.start.x);
        edgeData.push_back(edge.start.y);
        edgeData.push_back(edge.start.z);
        edgeData.push_back(edge.color.r);
        edgeData.push_back(edge.color.g);
        edgeData.push_back(edge.color.b);
        edgeData.push_back(edge.color.a);

        edgeData.push_back(edge.end.x);
        edgeData.push_back(edge.end.y);
        edgeData.push_back(edge.end.z);
        edgeData.push_back(edge.color.r);
        edgeData.push_back(edge.color.g);
        edgeData.push_back(edge.color.b);
        edgeData.push_back(edge.color.a);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_edgeVBO);
    glBufferData(GL_ARRAY_BUFFER, edgeData.size() * sizeof(float), edgeData.data(), GL_DYNAMIC_DRAW);

    m_edgeShader.use();
    float aspect = static_cast<float>(width()) / static_cast<float>(height());
    m_edgeShader.setMat4("view", m_camera.getViewMatrix());
    m_edgeShader.setMat4("projection", m_camera.getProjectionMatrix(aspect));

    glBindVertexArray(m_edgeVAO);
    glLineWidth(1.0f);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(edges.size() * 2));
}

void GlRenderer::renderParticles() {
    // Simplified particle rendering
}

void GlRenderer::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_mousePressed = true;
        m_lastMousePos = event->pos();

        if (m_nodeClickCallback) {
            pickNode(event->pos().x(), event->pos().y());
        }
    }
}

void GlRenderer::mouseMoveEvent(QMouseEvent* event) {
    if (m_mousePressed) {
        float xOffset = static_cast<float>(event->pos().x() - m_lastMousePos.x());
        float yOffset = static_cast<float>(m_lastMousePos.y() - event->pos().y());
        m_camera.processMouseMovement(xOffset, yOffset);
        m_lastMousePos = event->pos();
        update();
    }
}

void GlRenderer::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_mousePressed = false;
    }
}

void GlRenderer::wheelEvent(QWheelEvent* event) {
    m_camera.processMouseScroll(static_cast<float>(event->angleDelta().y()) / 120.0f);
    update();
}

void GlRenderer::pickNode(int x, int y) {
    // Simplified ray-based picking
}

} // namespace xray::renderer
