#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QTimer>
#include <unordered_map>

namespace xray::gui {

class ProcessView : public QWidget {
    Q_OBJECT

public:
    explicit ProcessView(QWidget* parent = nullptr);

    void updateProcessList();
    void addProcess(uint32_t pid, const QString& name, double cpu, double mem, uint32_t threads);
    void removeProcess(uint32_t pid);
    void updateProcess(uint32_t pid, double cpu, double mem);

signals:
    void processSelected(uint32_t pid);
    void processKilled(uint32_t pid);

private:
    void setupUi();
    void onItemClicked();
    void onContextMenu(const QPoint& pos);
    void refreshProcessList();

    QTableWidget* m_table{nullptr};
    QTimer* m_refreshTimer{nullptr};
    std::unordered_map<uint32_t, int> m_pidToRow;
};

} // namespace xray::gui
