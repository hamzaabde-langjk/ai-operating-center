#pragma once

#include <QWidget>
#include <QTimer>
#include <memory>
#include <atomic>

class QTableWidget;
class QProgressBar;
class QLabel;

namespace xray::gui {

class Dashboard : public QWidget {
    Q_OBJECT

public:
    explicit Dashboard(QWidget* parent = nullptr);

    void updateCpuUsage(double usage);
    void updateMemoryUsage(double usage);
    void updateDiskUsage(double usage);
    void updateNetworkStats(uint64_t rx, uint64_t tx);
    void updateEventRate(uint64_t eventsPerSecond);

signals:
    void alertTriggered(const QString& message);

private:
    void setupUi();
    void updateStats();

    QProgressBar* m_cpuBar{nullptr};
    QProgressBar* m_memBar{nullptr};
    QProgressBar* m_diskBar{nullptr};
    QLabel* m_netRxLabel{nullptr};
    QLabel* m_netTxLabel{nullptr};
    QLabel* m_eventRateLabel{nullptr};
    QTableWidget* m_topProcesses{nullptr};
    QTableWidget* m_recentAlerts{nullptr};

    QTimer* m_updateTimer{nullptr};

    std::atomic<double> m_cpuUsage{0.0};
    std::atomic<double> m_memUsage{0.0};
    std::atomic<double> m_diskUsage{0.0};
    std::atomic<uint64_t> m_netRx{0};
    std::atomic<uint64_t> m_netTx{0};
    std::atomic<uint64_t> m_eventRate{0};
};

} // namespace xray::gui
