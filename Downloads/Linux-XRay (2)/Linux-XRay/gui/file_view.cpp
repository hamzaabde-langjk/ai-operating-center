#include "gui/file_view.hpp"
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>

namespace xray::gui {

FileView::FileView(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);

    auto* searchLayout = new QHBoxLayout();
    auto* searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Search files...");
    auto* searchBtn = new QPushButton("Search", this);
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchBtn);
    layout->addLayout(searchLayout);

    auto* tree = new QTreeWidget(this);
    tree->setHeaderLabels({"Path", "Type", "Size", "Modified", "Accessed By"});
    tree->header()->setStretchLastSection(true);
    layout->addWidget(tree);

    // Add sample data
    auto* root = new QTreeWidgetItem(tree);
    root->setText(0, "/etc");
    root->setText(1, "Directory");

    auto* child = new QTreeWidgetItem(root);
    child->setText(0, "/etc/passwd");
    child->setText(1, "File");
    child->setText(2, "2.8 KB");
}

} // namespace xray::gui
