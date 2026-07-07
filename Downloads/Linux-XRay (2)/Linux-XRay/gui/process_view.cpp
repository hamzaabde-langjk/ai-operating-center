#include "gui/process_view.hpp"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>
#include <dirent.h>

namespace xray::gui {

ProcessView::ProcessView(QWidget* parent) : QWidget(parent) {
    setupUi();

    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &ProcessView::refreshProcessList);
    m_refreshTimer->start(2000);

    refreshProcessList();
}

void ProcessView::setupUi() {
    auto* layout = new QVBoxLayout(this);

    m_table = new QTableWidget(0, 8, this);
    m_table->setHorizontalHeaderLabels({
        "PID", "PPID", "Name", "CPU%", "MEM%", "Threads", "User", "Status"
    });
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    m_table->setSortingEnabled(true);

    connect(m_table, &QTableWidget::itemClicked, this, &ProcessView::onItemClicked);
    connect(m_table, &QTableWidget::customContextMenuRequested, 
            this, &ProcessView::onContextMenu);

    layout->addWidget(m_table);
}

void ProcessView::refreshProcessList() {
    // Read from /proc
    DIR* dir = opendir("/proc");
    if (!dir) return;

    struct dirent* entry;
    std::vector<std::tuple<uint32_t, uint32_t, QString, double, double, uint32_t, QString>> processes;

    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type != DT_DIR) continue;

        char* endptr;
        uint32_t pid = static_cast<uint32_t>(strtoul(entry->d_name, &endptr, 10));
        if (*endptr != '\0') continue;

        std::string statPath = std::string("/proc/") + entry->d_name + "/stat";
        std::ifstream statFile(statPath);
        if (!statFile.is_open()) continue;

        std::string line;
        std::getline(statFile, line);
        std::istringstream iss(line);

        uint32_t ppid = 0;
        std::string comm;
        char state;

        iss >> pid; // pid (already have it)
        iss >> comm; // comm in parentheses
        iss >> state; // state
        iss >> ppid; // ppid

        // Clean comm
        if (comm.front() == '(') comm = comm.substr(1);
        if (comm.back() == ')') comm.pop_back();

        // Read status for UID
        std::string statusPath = std::string("/proc/") + entry->d_name + "/status";
        std::ifstream statusFile(statusPath);
        QString user = "unknown";
        if (statusFile.is_open()) {
            std::string statusLine;
            while (std::getline(statusFile, statusLine)) {
                if (statusLine.find("Uid:") == 0) {
                    std::istringstream uidIss(statusLine);
                    std::string label;
                    uint32_t uid;
                    uidIss >> label >> uid;
                    user = QString::number(uid);
                    break;
                }
            }
        }

        // Read thread count
        std::string taskPath = std::string("/proc/") + entry->d_name + "/task";
        DIR* taskDir = opendir(taskPath.c_str());
        uint32_t threads = 0;
        if (taskDir) {
            struct dirent* taskEntry;
            while ((taskEntry = readdir(taskDir)) != nullptr) {
                if (taskEntry->d_type == DT_DIR) {
                    char* tEnd;
                    strtoul(taskEntry->d_name, &tEnd, 10);
                    if (*tEnd == '\0') threads++;
                }
            }
            closedir(taskDir);
        }

        processes.emplace_back(pid, ppid, QString::fromStdString(comm), 
                              0.0, 0.0, threads, user);
    }
    closedir(dir);

    // Update table
    m_table->setRowCount(0);
    m_pidToRow.clear();

    int row = 0;
    for (const auto& [pid, ppid, name, cpu, mem, threads, user] : processes) {
        m_table->insertRow(row);
        m_pidToRow[pid] = row;

        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(pid)));
        m_table->setItem(row, 1, new QTableWidgetItem(QString::number(ppid)));
        m_table->setItem(row, 2, new QTableWidgetItem(name));
        m_table->setItem(row, 3, new QTableWidgetItem(QString::number(cpu, 'f', 1)));
        m_table->setItem(row, 4, new QTableWidgetItem(QString::number(mem, 'f', 1)));
        m_table->setItem(row, 5, new QTableWidgetItem(QString::number(threads)));
        m_table->setItem(row, 6, new QTableWidgetItem(user));
        m_table->setItem(row, 7, new QTableWidgetItem("Running"));

        row++;
    }
}

void ProcessView::onItemClicked() {
    auto* item = m_table->currentItem();
    if (!item) return;

    int row = item->row();
    auto* pidItem = m_table->item(row, 0);
    if (pidItem) {
        uint32_t pid = pidItem->text().toUInt();
        emit processSelected(pid);
    }
}

void ProcessView::onContextMenu(const QPoint& pos) {
    auto* item = m_table->itemAt(pos);
    if (!item) return;

    QMenu menu(this);
    auto* killAction = menu.addAction("Kill Process");
    auto* inspectAction = menu.addAction("Inspect");
    auto* propertiesAction = menu.addAction("Properties");

    auto* selected = menu.exec(m_table->viewport()->mapToGlobal(pos));
    if (selected == killAction) {
        int row = item->row();
        auto* pidItem = m_table->item(row, 0);
        if (pidItem) {
            uint32_t pid = pidItem->text().toUInt();
            auto reply = QMessageBox::question(this, "Kill Process",
                QString("Are you sure you want to kill process %1?").arg(pid),
                QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                emit processKilled(pid);
            }
        }
    } else if (selected == inspectAction) {
        onItemClicked();
    }
}

void ProcessView::addProcess(uint32_t pid, const QString& name, double cpu, double mem, uint32_t threads) {
    // Used for eBPF-driven updates
}

void ProcessView::removeProcess(uint32_t pid) {
    auto it = m_pidToRow.find(pid);
    if (it != m_pidToRow.end()) {
        m_table->removeRow(it->second);
        m_pidToRow.erase(it);
    }
}

void ProcessView::updateProcess(uint32_t pid, double cpu, double mem) {
    auto it = m_pidToRow.find(pid);
    if (it != m_pidToRow.end()) {
        m_table->item(it->second, 3)->setText(QString::number(cpu, 'f', 1));
        m_table->item(it->second, 4)->setText(QString::number(mem, 'f', 1));
    }
}

} // namespace xray::gui
