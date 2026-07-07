#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <memory>
#include <thread>

namespace xray::engine { class EventBus; class EventDispatcher; }
namespace xray::graph { class Graph; }
namespace xray::database { class Database; }
namespace xray::security { class SecurityAnalyzer; }
namespace xray::renderer { class GlRenderer; class Scene; }

namespace xray::gui {

class Dashboard;
class ProcessView;
class InspectorPanel;
class TimelineWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void initialize();
    void shutdown();

private slots:
    void onProcessSelected(uint32_t pid);
    void onSecurityAlert(const QString& title, const QString& description);
    void onTimelineSeek(qint64 timestamp);
    void onExportData();
    void onSettings();
    void onAbout();

private:
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupCentralWidget();
    void connectSignals();
    void startMonitoring();
    void stopMonitoring();

    QTabWidget* m_tabWidget{nullptr};
    Dashboard* m_dashboard{nullptr};
    ProcessView* m_processView{nullptr};
    InspectorPanel* m_inspectorPanel{nullptr};
    TimelineWidget* m_timelineWidget{nullptr};
    GlRenderer* m_3dViewport{nullptr};

    std::shared_ptr<engine::EventBus> m_eventBus;
    std::shared_ptr<engine::EventDispatcher> m_dispatcher;
    std::shared_ptr<graph::Graph> m_graph;
    std::shared_ptr<database::Database> m_database;
    std::shared_ptr<security::SecurityAnalyzer> m_securityAnalyzer;
    std::shared_ptr<renderer::Scene> m_scene;

    std::thread m_monitorThread;
    std::atomic<bool> m_monitoring{false};
};

} // namespace xray::gui
