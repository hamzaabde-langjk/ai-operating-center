#include "gui/main_window.hpp"
#include "gui/dashboard.hpp"
#include "gui/process_view.hpp"
#include "gui/inspector_panel.hpp"
#include "gui/timeline_widget.hpp"
#include "gui/3d_viewport.hpp"
#include "engine/event_bus.hpp"
#include "engine/event_dispatcher.hpp"
#include "graph/graph.hpp"
#include "database/database.hpp"
#include "security/security_analyzer.hpp"
#include "renderer/scene.hpp"
#include "renderer/gl_renderer.hpp"

#include <spdlog/spdlog.h>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>
#include <QIcon>
#include <QLabel>
#include <QProgressBar>

namespace xray::gui {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Linux X-Ray Vision");
    setMinimumSize(1400, 900);
    resize(1600, 1000);
}

MainWindow::~MainWindow() {
    shutdown();
}

void MainWindow::initialize() {
    spdlog::info("Initializing MainWindow");

    // Initialize core components
    m_eventBus = std::make_shared<engine::EventBus>();
    m_dispatcher = std::make_shared<engine::EventDispatcher>(m_eventBus);
    m_graph = std::make_shared<graph::Graph>();
    m_database = std::make_shared<database::Database>();
    m_securityAnalyzer = std::make_shared<security::SecurityAnalyzer>();
    m_scene = std::make_shared<renderer::Scene>();

    // Initialize database
    if (!m_database->initialize()) {
        spdlog::error("Failed to initialize database");
    }

    // Setup UI
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupCentralWidget();
    connectSignals();

    // Start event bus
    m_eventBus->start();

    // Start monitoring
    startMonitoring();

    spdlog::info("MainWindow initialized");
}

void MainWindow::shutdown() {
    stopMonitoring();
    if (m_eventBus) {
        m_eventBus->stop();
    }
    spdlog::info("MainWindow shutdown complete");
}

void MainWindow::setupMenuBar() {
    auto* fileMenu = menuBar()->addMenu("&File");

    auto* exportAction = fileMenu->addAction("&Export...");
    exportAction->setShortcut(QKeySequence::SaveAs);
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportData);

    fileMenu->addSeparator();

    auto* quitAction = fileMenu->addAction("&Quit");
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    auto* viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction("&Dashboard");
    viewMenu->addAction("&Processes");
    viewMenu->addAction("&Network");
    viewMenu->addAction("&Files");
    viewMenu->addAction("&Security");
    viewMenu->addAction("&3D World");

    auto* toolsMenu = menuBar()->addMenu("&Tools");
    auto* settingsAction = toolsMenu->addAction("&Settings");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettings);

    auto* helpMenu = menuBar()->addMenu("&Help");
    auto* aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupToolBar() {
    auto* toolBar = addToolBar("Main");
    toolBar->setMovable(false);

    auto* recordAction = toolBar->addAction("Record");
    auto* replayAction = toolBar->addAction("Replay");
    auto* snapshotAction = toolBar->addAction("Snapshot");
    toolBar->addSeparator();
    auto* aiAction = toolBar->addAction("AI Analysis");
    auto* securityAction = toolBar->addAction("Security");
}

void MainWindow::setupStatusBar() {
    auto* status = statusBar();
    status->showMessage("Ready");

    auto* cpuLabel = new QLabel("CPU: 0%");
    auto* memLabel = new QLabel("MEM: 0%");
    auto* eventLabel = new QLabel("Events: 0/s");

    status->addPermanentWidget(cpuLabel);
    status->addPermanentWidget(memLabel);
    status->addPermanentWidget(eventLabel);
}

void MainWindow::setupCentralWidget() {
    m_tabWidget = new QTabWidget(this);

    // Dashboard tab
    m_dashboard = new Dashboard(this);
    m_tabWidget->addTab(m_dashboard, "Dashboard");

    // Process view tab
    m_processView = new ProcessView(this);
    m_tabWidget->addTab(m_processView, "Processes");

    // 3D World tab
    m_3dViewport = new GlRenderer(this);
    m_3dViewport->setScene(m_scene);
    m_tabWidget->addTab(m_3dViewport, "3D World");

    // Inspector panel (docked)
    m_inspectorPanel = new InspectorPanel(this);
    addDockWidget(Qt::RightDockWidgetArea, m_inspectorPanel);

    // Timeline (docked bottom)
    m_timelineWidget = new TimelineWidget(this);
    addDockWidget(Qt::BottomDockWidgetArea, m_timelineWidget);

    setCentralWidget(m_tabWidget);
}

void MainWindow::connectSignals() {
    // Connect process selection to inspector
    connect(m_processView, &ProcessView::processSelected,
            this, &MainWindow::onProcessSelected);

    // Connect security alerts
    // connect(m_securityAnalyzer.get(), ...); // Would use signal/slot wrapper

    // Connect timeline
    connect(m_timelineWidget, &TimelineWidget::seekRequested,
            this, &MainWindow::onTimelineSeek);
}

void MainWindow::startMonitoring() {
    m_monitoring = true;
    m_monitorThread = std::thread([this]() {
        while (m_monitoring) {
            // Simulate event generation for demo
            // In production, this would read from eBPF ring buffers
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

void MainWindow::stopMonitoring() {
    m_monitoring = false;
    if (m_monitorThread.joinable()) {
        m_monitorThread.join();
    }
}

void MainWindow::onProcessSelected(uint32_t pid) {
    m_inspectorPanel->inspectProcess(pid);
}

void MainWindow::onSecurityAlert(const QString& title, const QString& description) {
    QMessageBox::warning(this, title, description);
}

void MainWindow::onTimelineSeek(qint64 timestamp) {
    spdlog::info("Timeline seek to {}", timestamp);
    // Load events from database around this timestamp
}

void MainWindow::onExportData() {
    QString fileName = QFileDialog::getSaveFileName(this, "Export Data", "",
        "JSON (*.json);;CSV (*.csv);;SQLite (*.db);;GraphML (*.graphml)");
    if (!fileName.isEmpty()) {
        spdlog::info("Exporting to {}", fileName.toStdString());
    }
}

void MainWindow::onSettings() {
    QMessageBox::information(this, "Settings", "Settings dialog not yet implemented.");
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About Linux X-Ray Vision",
        "<h2>Linux X-Ray Vision 0.1.0</h2>"
        "<p>A professional open-source Linux system monitor with 3D visualization.</p>"
        "<p>Built with eBPF, Qt6, and OpenGL.</p>"
        "<p>License: MIT</p>");
}

} // namespace xray::gui
