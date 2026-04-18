#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QRect>
#include <windows.h>
#include <physicalmonitorenumerationapi.h>
#include <highlevelmonitorconfigurationapi.h>
#include <lowlevelmonitorconfigurationapi.h>

struct MonitorInfo {
    int id;
    QString name;
    QRect rect;
    bool isActive;
    bool isPowerOn;
    HMONITOR hMonitor;
    QVector<HANDLE> physicalHandles;
    LUID adapterId;
    UINT32 targetId;
};

class ScreenManager : public QObject
{
    Q_OBJECT
public:
    explicit ScreenManager(QObject *parent = nullptr);
    ~ScreenManager();

    QVector<MonitorInfo> getMonitors();
    bool setMonitorPower(int id, bool on);
    bool setAllMonitorsPower(bool on);
    bool setMonitorActive(int id, bool active);
    
    void refresh();

private:
    void cleanupPhysicalHandles();
    QVector<MonitorInfo> m_monitors;
    
    // Windows Display Config structures
    struct SavedConfig {
        DISPLAYCONFIG_PATH_INFO* pathArray = nullptr;
        DISPLAYCONFIG_MODE_INFO* modeArray = nullptr;
        UINT32 pathCount = 0;
        UINT32 modeCount = 0;
    } m_savedConfig;

    void saveDisplayConfig();
    void freeSavedConfig();
};

#endif // SCREENMANAGER_H
