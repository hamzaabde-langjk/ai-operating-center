#include <QApplication>
#include "gui/main_window.hpp"
#include <spdlog/spdlog.h>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Linux X-Ray Vision");
    app.setOrganizationName("xray-team");

    spdlog::set_level(spdlog::level::info);
    spdlog::info("Starting Linux X-Ray Vision GUI");

    xray::gui::MainWindow window;
    window.initialize();
    window.show();

    int result = app.exec();
    window.shutdown();
    return result;
}
