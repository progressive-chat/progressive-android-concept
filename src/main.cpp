#include <QApplication>

#include "app/main_window.hpp"
#include "app/application.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    progressive::Application &application = progressive::Application::instance();
    application.loadSettings();
    application.startProtocolManager();

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
