#include "gui/inspector_panel.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QTextEdit>
#include <QGroupBox>
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace xray::gui {

InspectorPanel::InspectorPanel(QWidget* parent) : QDockWidget("Inspector", parent) {
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    setupUi();
}

void InspectorPanel::setupUi() {
    auto* container = new QWidget(this);
    auto* layout = new QVBoxLayout(container);

    m_tabWidget = new QTabWidget(container);

    // General info tab
    auto* generalTab = new QWidget(m_tabWidget);
    auto* generalLayout = new QGridLayout(generalTab);

    generalLayout->addWidget(new QLabel("PID:"), 0, 0);
    m_pidLabel = new QLabel("-", generalTab);
    generalLayout->addWidget(m_pidLabel, 0, 1);

    generalLayout->addWidget(new QLabel("PPID:"), 1, 0);
    m_ppidLabel = new QLabel("-", generalTab);
    generalLayout->addWidget(m_ppidLabel, 1, 1);

    generalLayout->addWidget(new QLabel("Name:"), 2, 0);
    m_nameLabel = new QLabel("-", generalTab);
    generalLayout->addWidget(m_nameLabel, 2, 1);

    generalLayout->addWidget(new QLabel("Command Line:"), 3, 0);
    m_cmdlineLabel = new QLabel("-", generalTab);
    m_cmdlineLabel->setWordWrap(true);
    generalLayout->addWidget(m_cmdlineLabel, 3, 1);

    generalLayout->addWidget(new QLabel("CPU Usage:"), 4, 0);
    m_cpuLabel = new QLabel("-", generalTab);
    generalLayout->addWidget(m_cpuLabel, 4, 1);

    generalLayout->addWidget(new QLabel("Memory Usage:"), 5, 0);
    m_memLabel = new QLabel("-", generalTab);
    generalLayout->addWidget(m_memLabel, 5, 1);

    generalLayout->addWidget(new QLabel("Threads:"), 6, 0);
    m_threadsLabel = new QLabel("-", generalTab);
    generalLayout->addWidget(m_threadsLabel, 6, 1);

    generalLayout->addWidget(new QLabel("UID:"), 7, 0);
    m_uidLabel = new QLabel("-", generalTab);
    generalLayout->addWidget(m_uidLabel, 7, 1);

    generalLayout->addWidget(new QLabel("GID:"), 8, 0);
    m_gidLabel = new QLabel("-", generalTab);
    generalLayout->addWidget(m_gidLabel, 8, 1);

    generalLayout->addWidget(new QLabel("CWD:"), 9, 0);
    m_cwdLabel = new QLabel("-", generalTab);
    generalLayout->addWidget(m_cwdLabel, 9, 1);

    generalLayout->addWidget(new QLabel("Executable:"), 10, 0);
    m_exeLabel = new QLabel("-", generalTab);
    generalLayout->addWidget(m_exeLabel, 10, 1);

    generalLayout->setRowStretch(11, 1);
    m_tabWidget->addTab(generalTab, "General");

    // Environment tab
    m_envTable = new QTableWidget(0, 2, m_tabWidget);
    m_envTable->setHorizontalHeaderLabels({"Variable", "Value"});
    m_envTable->horizontalHeader()->setStretchLastSection(true);
    m_tabWidget->addTab(m_envTable, "Environment");

    // Open files tab
    m_filesTable = new QTableWidget(0, 3, m_tabWidget);
    m_filesTable->setHorizontalHeaderLabels({"FD", "Type", "Path"});
    m_filesTable->horizontalHeader()->setStretchLastSection(true);
    m_tabWidget->addTab(m_filesTable, "Open Files");

    // Network tab
    m_networkTable = new QTableWidget(0, 5, m_tabWidget);
    m_networkTable->setHorizontalHeaderLabels({"Protocol", "Local", "Remote", "State", "Inode"});
    m_networkTable->horizontalHeader()->setStretchLastSection(true);
    m_tabWidget->addTab(m_networkTable, "Network");

    // Capabilities tab
    m_capsText = new QTextEdit(m_tabWidget);
    m_capsText->setReadOnly(true);
    m_tabWidget->addTab(m_capsText, "Capabilities");

    // Namespaces tab
    m_nsText = new QTextEdit(m_tabWidget);
    m_nsText->setReadOnly(true);
    m_tabWidget->addTab(m_nsText, "Namespaces");

    // Security tab
    m_securityText = new QTextEdit(m_tabWidget);
    m_securityText->setReadOnly(true);
    m_tabWidget->addTab(m_securityText, "Security");

    layout->addWidget(m_tabWidget);
    setWidget(container);
}

void InspectorPanel::inspectProcess(uint32_t pid) {
    m_currentPid = pid;
    loadProcessInfo(pid);
    loadProcessEnvironment(pid);
    loadOpenFiles(pid);
    loadNetworkConnections(pid);
}

void InspectorPanel::loadProcessInfo(uint32_t pid) {
    m_pidLabel->setText(QString::number(pid));

    std::string statPath = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream statFile(statPath);
    if (statFile.is_open()) {
        std::string line;
        std::getline(statFile, line);
        std::istringstream iss(line);

        uint32_t p;
        std::string comm;
        char state;
        uint32_t ppid;

        iss >> p >> comm >> state >> ppid;

        if (comm.front() == '(') comm = comm.substr(1);
        if (comm.back() == ')') comm.pop_back();

        m_ppidLabel->setText(QString::number(ppid));
        m_nameLabel->setText(QString::fromStdString(comm));
    }

    // Command line
    std::string cmdlinePath = "/proc/" + std::to_string(pid) + "/cmdline";
    std::ifstream cmdlineFile(cmdlinePath);
    if (cmdlineFile.is_open()) {
        std::string cmdline;
        std::getline(cmdlineFile, cmdline);
        // Replace null chars with spaces
        std::replace(cmdline.begin(), cmdline.end(), '\0', ' ');
        m_cmdlineLabel->setText(QString::fromStdString(cmdline));
    }

    // Status
    std::string statusPath = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream statusFile(statusPath);
    if (statusFile.is_open()) {
        std::string line;
        while (std::getline(statusFile, line)) {
            if (line.find("Uid:") == 0) {
                std::istringstream iss(line);
                std::string label;
                uint32_t uid;
                iss >> label >> uid;
                m_uidLabel->setText(QString::number(uid));
            } else if (line.find("Gid:") == 0) {
                std::istringstream iss(line);
                std::string label;
                uint32_t gid;
                iss >> label >> gid;
                m_gidLabel->setText(QString::number(gid));
            } else if (line.find("Threads:") == 0) {
                std::istringstream iss(line);
                std::string label;
                uint32_t threads;
                iss >> label >> threads;
                m_threadsLabel->setText(QString::number(threads));
            }
        }
    }

    // CWD
    char cwdBuf[4096];
    std::string cwdLink = "/proc/" + std::to_string(pid) + "/cwd";
    ssize_t len = readlink(cwdLink.c_str(), cwdBuf, sizeof(cwdBuf) - 1);
    if (len != -1) {
        cwdBuf[len] = '\0';
        m_cwdLabel->setText(QString(cwdBuf));
    }

    // Exe
    char exeBuf[4096];
    std::string exeLink = "/proc/" + std::to_string(pid) + "/exe";
    len = readlink(exeLink.c_str(), exeBuf, sizeof(exeBuf) - 1);
    if (len != -1) {
        exeBuf[len] = '\0';
        m_exeLabel->setText(QString(exeBuf));
    }

    // Memory (from statm)
    std::string statmPath = "/proc/" + std::to_string(pid) + "/statm";
    std::ifstream statmFile(statmPath);
    if (statmFile.is_open()) {
        unsigned long size, resident;
        statmFile >> size >> resident;
        double memMB = resident * 4096.0 / (1024.0 * 1024.0);
        m_memLabel->setText(QString("%1 MB").arg(memMB, 0, 'f', 1));
    }
}

void InspectorPanel::loadProcessEnvironment(uint32_t pid) {
    m_envTable->setRowCount(0);

    std::string envPath = "/proc/" + std::to_string(pid) + "/environ";
    std::ifstream envFile(envPath);
    if (!envFile.is_open()) return;

    std::string content((std::istreambuf_iterator<char>(envFile)),
                        std::istreambuf_iterator<char>());

    size_t start = 0;
    int row = 0;
    while (start < content.size()) {
        size_t end = content.find('\0', start);
        if (end == std::string::npos) end = content.size();

        std::string env = content.substr(start, end - start);
        size_t eqPos = env.find('=');
        if (eqPos != std::string::npos) {
            m_envTable->insertRow(row);
            m_envTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(env.substr(0, eqPos))));
            m_envTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(env.substr(eqPos + 1))));
            row++;
        }
        start = end + 1;
    }
}

void InspectorPanel::loadOpenFiles(uint32_t pid) {
    m_filesTable->setRowCount(0);

    std::string fdPath = "/proc/" + std::to_string(pid) + "/fd";
    if (!std::filesystem::exists(fdPath)) return;

    int row = 0;
    for (const auto& entry : std::filesystem::directory_iterator(fdPath)) {
        char linkBuf[4096];
        ssize_t len = readlink(entry.path().c_str(), linkBuf, sizeof(linkBuf) - 1);
        if (len == -1) continue;
        linkBuf[len] = '\0';

        m_filesTable->insertRow(row);
        m_filesTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(entry.path().filename().string())));

        std::string path(linkBuf);
        QString type = "file";
        if (path.find("socket:") == 0) type = "socket";
        else if (path.find("pipe:") == 0) type = "pipe";
        else if (path == "/dev/null") type = "null";
        else if (path == "/dev/zero") type = "zero";
        else if (path == "/dev/urandom") type = "urandom";
        else if (path.find("anon_inode:") == 0) type = "anon_inode";

        m_filesTable->setItem(row, 1, new QTableWidgetItem(type));
        m_filesTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(path)));
        row++;
    }
}

void InspectorPanel::loadNetworkConnections(uint32_t pid) {
    m_networkTable->setRowCount(0);

    // Read /proc/pid/net/tcp and /proc/pid/net/udp
    std::vector<std::string> protocols = {"tcp", "udp"};
    int row = 0;

    for (const auto& proto : protocols) {
        std::string netPath = "/proc/" + std::to_string(pid) + "/net/" + proto;
        std::ifstream netFile(netPath);
        if (!netFile.is_open()) continue;

        std::string line;
        std::getline(netFile, line); // Skip header

        while (std::getline(netFile, line)) {
            std::istringstream iss(line);
            std::string sl, localAddr, remAddr, stateStr;
            unsigned int state;

            iss >> sl >> localAddr >> remAddr >> stateStr;

            // Parse hex addresses
            auto parseAddr = [](const std::string& hex) -> QString {
                if (hex.size() < 9) return "0.0.0.0:0";
                unsigned int ip = std::stoul(hex.substr(0, 8), nullptr, 16);
                unsigned int port = std::stoul(hex.substr(9), nullptr, 16);
                return QString("%1.%2.%3.%4:%5")
                    .arg(ip & 0xFF).arg((ip >> 8) & 0xFF)
                    .arg((ip >> 16) & 0xFF).arg((ip >> 24) & 0xFF)
                    .arg(port);
            };

            QString stateName;
            switch (state) {
                case 1: stateName = "ESTABLISHED"; break;
                case 2: stateName = "SYN_SENT"; break;
                case 3: stateName = "SYN_RECV"; break;
                case 4: stateName = "FIN_WAIT1"; break;
                case 5: stateName = "FIN_WAIT2"; break;
                case 6: stateName = "TIME_WAIT"; break;
                case 7: stateName = "CLOSE"; break;
                case 8: stateName = "CLOSE_WAIT"; break;
                case 9: stateName = "LAST_ACK"; break;
                case 10: stateName = "LISTEN"; break;
                case 11: stateName = "CLOSING"; break;
                default: stateName = "UNKNOWN"; break;
            }

            m_networkTable->insertRow(row);
            m_networkTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(proto)));
            m_networkTable->setItem(row, 1, new QTableWidgetItem(parseAddr(localAddr)));
            m_networkTable->setItem(row, 2, new QTableWidgetItem(parseAddr(remAddr)));
            m_networkTable->setItem(row, 3, new QTableWidgetItem(stateName));
            m_networkTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(sl)));
            row++;
        }
    }
}

void InspectorPanel::inspectFile(const QString& path) {
    // TODO: Implement file inspection
}

void InspectorPanel::inspectNetworkConnection(uint32_t localPort, uint32_t remotePort) {
    // TODO: Implement network connection inspection
}

void InspectorPanel::clear() {
    m_currentPid = 0;
    m_pidLabel->setText("-");
    m_ppidLabel->setText("-");
    m_nameLabel->setText("-");
    m_cmdlineLabel->setText("-");
    m_cpuLabel->setText("-");
    m_memLabel->setText("-");
    m_threadsLabel->setText("-");
    m_uidLabel->setText("-");
    m_gidLabel->setText("-");
    m_cwdLabel->setText("-");
    m_exeLabel->setText("-");
    m_envTable->setRowCount(0);
    m_filesTable->setRowCount(0);
    m_networkTable->setRowCount(0);
    m_capsText->clear();
    m_nsText->clear();
    m_securityText->clear();
}

} // namespace xray::gui
