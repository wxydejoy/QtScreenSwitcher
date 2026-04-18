#include <QApplication>
#include "MainWindow.h"
#include "TrayIcon.h"
#include <QStyleFactory>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("QtScreenSwitcher");
    a.setOrganizationName("QtScreenSwitcher");
    a.setQuitOnLastWindowClosed(false);

    // Set fusion style for a consistent modern look
    a.setStyle(QStyleFactory::create("Fusion"));

    // Load stylesheet from embedded resources so it works from any launch directory.
    QFile styleFile(":/ui/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        a.setStyleSheet(styleFile.readAll());
    }

    MainWindow w;
    TrayIcon tray(&w);
    
    tray.show();
    w.show();

    return a.exec();
}
