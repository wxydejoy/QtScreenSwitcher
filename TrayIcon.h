#ifndef TRAYICON_H
#define TRAYICON_H

#include <QSystemTrayIcon>
#include <QMenu>

class MainWindow;

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    explicit TrayIcon(MainWindow *mainWindow, QObject *parent = nullptr);

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    MainWindow *m_mainWindow;
    QMenu *m_menu;
};

#endif // TRAYICON_H
