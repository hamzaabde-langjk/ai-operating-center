#include "gui/dashboard.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTableWidget>
#include <QProgressBar>
#include <QLabel>
#include <QHeaderView>
#include <spdlog/spdlog.h>

namespace xray::gui {

Dashboard::Dashboard(QWidget* parent) : QWidget(parent) {
    setupUi();

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &Dashboard::updateStats);
    m_updateTimer->start(1000);
}

void Dashboard::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);

    // System metrics group
    auto* metricsGroup = new QGroupBox("System Metrics", this);
    auto* metricsLayout = new QGridLayout(metricsGroup);

    metricsLayout->addWidget(new QLabel("CPU:"), 0, 0);
    m_cpuBar = new QProgressBar(this);
    m_cpuBar->setRange(0, 100);
    metricsLayout->addWidget(m_cpuBar, 0, 1);

    metricsLayout->addWidget(new QLabel("Memory:"), 1, 0);
    m_memBar = new QProgressBar(this);
    m_memBar->setRange(0, 100);
    metricsLayout->addWidget(m_memBar, 1, 1);

    metricsLayout->addWidget(new QLabel("Disk:"), 2, 0);
    m_diskBar = new QProgressBar(this);
    m_diskBar->setRange(0, 100);
    metricsLayout->addWidget(m_diskBar, 2, 1);

    metricsLayout->addWidget(new QLabel("Network RX:"), 3, 0);
    m_netRxLabel = new QLabel("0 B/s", this);
    metricsLayout->addWidget(m_netRxLabel, 3, 1);

    metricsLayout->addWidget(new QLabel("Network TX:"), 4, 0);
    m_netTxLabel = new QLabel("0 B/s", this);
    metricsLayout->addWidget(m_netTxLabel, 4, 1);

    metricsLayout->addWidget(new QLabel("Event Rate:"), 5, 0);
    m_eventRateLabel = new QLabel("0 events/s", this);
    metricsLayout->addWidget(m_eventRateLabel, 5, 1);

    mainLayout->addWidget(metricsGroup);

    // Top processes
    auto* processGroup = new QGroupBox("Top Processes", this);
    auto* processLayout = new QVBoxLayout(processGroup);
    m_topProcesses = new QTableWidget(0, 5, this);
    m_topProcesses->setHorizontalHeaderLabels({"PID", "Name", "CPU%", "MEM%", "Status"});
    m_topProcesses->horizontalHeader()->setStretchLastSection(true);
    m_topProcesses->setSelectionBehavior(QAbstractItemView::SelectRows);
    processLayout->addWidget(m_topProcesses);
    mainLayout->addWidget(processGroup);

    // Recent alerts
    auto* alertGroup = new QGroupBox("Recent Security Alerts", this);
    auto* alertLayout = new QVBoxLayout(alertGroup);
    m_recentAlerts = new QTableWidget(0, 4, this);
    m_recentAlerts->setHorizontalHeaderLabels({"Time", "Level", "Type", "Description"});
    m_recentAlerts->horizontalHeader()->setStretchLastSection(true);
    alertLayout->addWidget(m_recentAlerts);
    mainLayout->addWidget(alertGroup);
}

void Dashboard::updateStats() {
    m_cpuBar->setValue(static_cast<int>(m_cpuUsage.load()));
    m_memBar->setValue(static_cast<int>(m_memUsage.load()));
    m_diskBar->setValue(static_cast<int>(m_diskUsage.load()));

    auto formatBytes = [](uint64_t bytes) -> QString {
        if (bytes < 1024) return QString("%1 B/s").arg(bytes);
        if (bytes < 1024 * 1024) return QString("%1 KB/s").arg(bytes / 1024.0, 0, 'f', 1);
        if (bytes < 1024 * 1024 * 1024) return QString("%1 MB/s").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
        return QString("%1 GB/s").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
    };

    m_netRxLabel->setText(formatBytes(m_netRx.load()));
    m_netTxLabel->setText(formatBytes(m_netTx.load()));
    m_eventRateLabel->setText(QString("%1 events/s").arg(m_eventRate.load()));
}

void Dashboard::updateCpuUsage(double usage) {
    m_cpuUsage.store(usage);
}

void Dashboard::updateMemoryUsage(double usage) {
    m_memUsage.store(usage);
}

void Dashboard::updateDiskUsage(double usage) {
    m_diskUsage.store(usage);
}

void Dashboard::updateNetworkStats(uint64_t rx, uint64_t tx) {
    m_netRx.store(rx);
    m_netTx.store(tx);
}

void Dashboard::updateEventRate(uint64_t eventsPerSecond) {
    m_eventRate.store(eventsPerSecond);
}

} // namespace xray::gui
