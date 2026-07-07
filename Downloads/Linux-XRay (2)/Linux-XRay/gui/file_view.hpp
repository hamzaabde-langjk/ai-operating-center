#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace xray::gui {

class FileView : public QWidget {
    Q_OBJECT
public:
    explicit FileView(QWidget* parent = nullptr);

    void setRootPath(const QString& path);
    void refresh();

signals:
    void fileSelected(const QString& path);
    void directorySelected(const QString& path);

private:
    void setupUi();
    void populateTree(const QString& path, QTreeWidgetItem* parent);

    QTreeWidget* m_treeWidget{nullptr};
    QLineEdit* m_pathEdit{nullptr};
    QPushButton* m_refreshBtn{nullptr};
    QString m_currentPath{"/"};
};

} // namespace xray::gui
