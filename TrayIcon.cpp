#include "TrayIcon.h"
#include "MainWindow.h"
#include <QApplication>
#include <QIcon>
#include <QStyle>

TrayIcon::TrayIcon(MainWindow *mainWindow, QObject *parent)
    : QSystemTrayIcon(parent), m_mainWindow(mainWindow)
{
    // Use a standard system icon as a placeholder
    setIcon(qApp->style()->standardIcon(QStyle::SP_ComputerIcon));
    setToolTip("Qt Screen Switcher");

    m_menu = new QMenu();
    
    QAction *showAction = m_menu->addAction("Show MainWindow");
    connect(showAction, &QAction::triggered, m_mainWindow, &MainWindow::show);

    m_menu->addSeparator();
    
    QAction *quitAction = m_menu->addAction("Quit");
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    setContextMenu(m_menu);

    connect(this, &QSystemTrayIcon::activated, this, &TrayIcon::onTrayIconActivated);
}

void TrayIcon::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        if (m_mainWindow->isVisible()) {
            m_mainWindow->hide();
        } else {
            m_mainWindow->show();
            m_mainWindow->raise();
            m_mainWindow->activateWindow();
        }
    }
}
