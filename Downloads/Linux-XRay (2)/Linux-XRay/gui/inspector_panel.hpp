#pragma once

#include <QDockWidget>
#include <cstdint>

class QTabWidget;
class QLabel;
class QTableWidget;
class QTextEdit;

namespace xray::gui {

class InspectorPanel : public QDockWidget {
    Q_OBJECT

public:
    explicit InspectorPanel(QWidget* parent = nullptr);

    void inspectProcess(uint32_t pid);
    void inspectFile(const QString& path);
    void inspectNetworkConnection(uint32_t localPort, uint32_t remotePort);
    void clear();

private:
    void setupUi();
    void loadProcessInfo(uint32_t pid);
    void loadProcessEnvironment(uint32_t pid);
    void loadOpenFiles(uint32_t pid);
    void loadNetworkConnections(uint32_t pid);

    QTabWidget* m_tabWidget{nullptr};

    // General info tab
    QLabel* m_pidLabel{nullptr};
    QLabel* m_ppidLabel{nullptr};
    QLabel* m_nameLabel{nullptr};
    QLabel* m_cmdlineLabel{nullptr};
    QLabel* m_cpuLabel{nullptr};
    QLabel* m_memLabel{nullptr};
    QLabel* m_threadsLabel{nullptr};
    QLabel* m_uidLabel{nullptr};
    QLabel* m_gidLabel{nullptr};
    QLabel* m_cwdLabel{nullptr};
    QLabel* m_exeLabel{nullptr};

    // Environment tab
    QTableWidget* m_envTable{nullptr};

    // Open files tab
    QTableWidget* m_filesTable{nullptr};

    // Network tab
    QTableWidget* m_networkTable{nullptr};

    // Capabilities tab
    QTextEdit* m_capsText{nullptr};

    // Namespaces tab
    QTextEdit* m_nsText{nullptr};

    // Security tab
    QTextEdit* m_securityText{nullptr};

    uint32_t m_currentPid{0};
};

} // namespace xray::gui
