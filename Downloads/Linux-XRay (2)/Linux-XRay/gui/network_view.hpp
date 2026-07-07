#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace xray::gui {

class NetworkView : public QWidget {
    Q_OBJECT
public:
    explicit NetworkView(QWidget* parent = nullptr);

    void refreshConnections();
    void filterConnections(const QString& filter);

signals:
    void connectionSelected(const QString& protocol, const QString& localAddr, const QString& remoteAddr);

private:
    void setupUi();
    void readConnections();

    QTableWidget* m_tableWidget{nullptr};
    QLineEdit* m_filterEdit{nullptr};
    QPushButton* m_refreshBtn{nullptr};
    QPushButton* m_filterBtn{nullptr};
};

} // namespace xray::gui
