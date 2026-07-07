#include "gui/network_view.hpp"
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>

namespace xray::gui {

NetworkView::NetworkView(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);

    auto* searchLayout = new QHBoxLayout();
    auto* searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Filter connections...");
    auto* filterBtn = new QPushButton("Filter", this);
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(filterBtn);
    layout->addLayout(searchLayout);

    auto* table = new QTableWidget(0, 7, this);
    table->setHorizontalHeaderLabels({
        "Protocol", "Local Address", "Local Port", "Remote Address", 
        "Remote Port", "State", "Process"
    });
    table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table);
}

} // namespace xray::gui
