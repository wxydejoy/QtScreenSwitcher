#include "ScreenManager.h"
#include <QDebug>
#include <QtGlobal>

ScreenManager::ScreenManager(QObject *parent) : QObject(parent)
{
    refresh();
}

ScreenManager::~ScreenManager()
{
    cleanupPhysicalHandles();
    freeSavedConfig();
}

void ScreenManager::cleanupPhysicalHandles()
{
    for (auto& monitor : m_monitors) {
        for (HANDLE h : monitor.physicalHandles) {
            DestroyPhysicalMonitor(h);
        }
        monitor.physicalHandles.clear();
    }
}

void ScreenManager::refresh()
{
    cleanupPhysicalHandles();
    m_monitors.clear();

    // 1. Get current display config (including inactive ones)
    UINT32 pathCount = 0;
    UINT32 modeCount = 0;
    if (GetDisplayConfigBufferSizes(QDC_ALL_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS) {
        return;
    }

    QVector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
    QVector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

    if (QueryDisplayConfig(QDC_ALL_PATHS, &pathCount, paths.data(), &modeCount, modes.data(), NULL) != ERROR_SUCCESS) {
        return;
    }

    // 2. Map paths to MonitorInfo
    for (UINT32 i = 0; i < pathCount; ++i) {
        // Filter: Only show monitors that are physically available/connected
        if (!paths[i].targetInfo.targetAvailable) {
            continue;
        }

        // De-duplicate: Some monitors might have multiple paths (e.g. historical modes)
        // We only care about unique target devices.
        bool duplicate = false;
        for (const auto& existing : m_monitors) {
            if (existing.adapterId.LowPart == paths[i].targetInfo.adapterId.LowPart &&
                existing.adapterId.HighPart == paths[i].targetInfo.adapterId.HighPart &&
                existing.targetId == paths[i].targetInfo.id) {
                
                // If we find a duplicate, prefer the active path if the current one is inactive
                if (!existing.isActive && (paths[i].flags & DISPLAYCONFIG_PATH_ACTIVE)) {
                    // Update existing to active (this shouldn't happen often with QDC_ALL_PATHS but just in case)
                }
                duplicate = true;
                break;
            }
        }
        if (duplicate) continue;

        MonitorInfo info;
        info.id = m_monitors.size();
        info.isActive = (paths[i].flags & DISPLAYCONFIG_PATH_ACTIVE);
        info.isPowerOn = true; // Default for inactive or if check fails
        info.adapterId = paths[i].targetInfo.adapterId;
        info.targetId = paths[i].targetInfo.id;
        
        // Get target name
        DISPLAYCONFIG_TARGET_DEVICE_NAME targetName;
        targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        targetName.header.size = sizeof(DISPLAYCONFIG_TARGET_DEVICE_NAME);
        targetName.header.adapterId = paths[i].targetInfo.adapterId;
        targetName.header.id = paths[i].targetInfo.id;
        
        if (DisplayConfigGetDeviceInfo(&targetName.header) == ERROR_SUCCESS) {
            info.name = QString::fromWCharArray(targetName.monitorFriendlyDeviceName);
        }
        
        if (info.name.isEmpty()) {
            info.name = QString("Display %1").arg(info.id + 1);
        }

        if (info.isActive) {
            UINT32 modeIdx = paths[i].sourceInfo.modeInfoIdx;
            if (modeIdx < modeCount) {
                info.rect = QRect(modes[modeIdx].sourceMode.position.x, 
                                  modes[modeIdx].sourceMode.position.y,
                                  modes[modeIdx].sourceMode.width, 
                                  modes[modeIdx].sourceMode.height);
            }
        }
        
        m_monitors.push_back(info);
    }
    
    // 3. For active monitors, try to get physical handles for DDC/CI
    EnumDisplayMonitors(NULL, NULL, [](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) -> BOOL {
        ScreenManager* self = reinterpret_cast<ScreenManager*>(dwData);
        QRect rect(lprcMonitor->left, lprcMonitor->top, 
                   lprcMonitor->right - lprcMonitor->left, 
                   lprcMonitor->bottom - lprcMonitor->top);
        
        for (auto& monitor : self->m_monitors) {
            if (monitor.isActive && monitor.rect == rect) {
                monitor.hMonitor = hMonitor;
                DWORD physicalCount = 0;
                if (GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &physicalCount)) {
                    LPPHYSICAL_MONITOR pPhysicalMonitors = (LPPHYSICAL_MONITOR)malloc(physicalCount * sizeof(PHYSICAL_MONITOR));
                    if (GetPhysicalMonitorsFromHMONITOR(hMonitor, physicalCount, pPhysicalMonitors)) {
                        for (DWORD i = 0; i < physicalCount; ++i) {
                            HANDLE h = pPhysicalMonitors[i].hPhysicalMonitor;
                            monitor.physicalHandles.push_back(h);
                            
                            // Check current power state (VCP 0xD6)
                            DWORD current = 0, maxVal = 0;
                            if (GetVCPFeatureAndVCPFeatureReply(h, 0xD6, NULL, &current, &maxVal)) {
                                if (current == 0x04 || current == 0x05) { // 0x04=Off, 0x05=DPMOff
                                    monitor.isPowerOn = false;
                                } else {
                                    monitor.isPowerOn = true;
                                }
                            }
                        }
                    }
                    free(pPhysicalMonitors);
                }
                break;
            }
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(this));

    saveDisplayConfig();
}

QVector<MonitorInfo> ScreenManager::getMonitors()
{
    return m_monitors;
}

bool ScreenManager::setMonitorPower(int id, bool on)
{
    if (id < 0 || id >= m_monitors.size()) return false;
    
    bool success = true;
    for (HANDLE h : m_monitors[id].physicalHandles) {
        if (!SetVCPFeature(h, 0xD6, on ? 0x01 : 0x04)) {
            success = false;
        }
    }
    return success;
}

bool ScreenManager::setAllMonitorsPower(bool on)
{
    return SendMessageW(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, on ? -1 : 2) != 0;
}

void ScreenManager::saveDisplayConfig()
{
    freeSavedConfig();
    
    UINT32 pathCount = 0;
    UINT32 modeCount = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS) {
        return;
    }

    m_savedConfig.pathArray = (DISPLAYCONFIG_PATH_INFO*)malloc(pathCount * sizeof(DISPLAYCONFIG_PATH_INFO));
    m_savedConfig.modeArray = (DISPLAYCONFIG_MODE_INFO*)malloc(modeCount * sizeof(DISPLAYCONFIG_MODE_INFO));
    m_savedConfig.pathCount = pathCount;
    m_savedConfig.modeCount = modeCount;

    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &m_savedConfig.pathCount, m_savedConfig.pathArray,
                           &m_savedConfig.modeCount, m_savedConfig.modeArray, NULL) != ERROR_SUCCESS) {
        freeSavedConfig();
    }
}

void ScreenManager::freeSavedConfig()
{
    if (m_savedConfig.pathArray) free(m_savedConfig.pathArray);
    if (m_savedConfig.modeArray) free(m_savedConfig.modeArray);
    m_savedConfig.pathArray = nullptr;
    m_savedConfig.modeArray = nullptr;
    m_savedConfig.pathCount = 0;
    m_savedConfig.modeCount = 0;
}

bool ScreenManager::setMonitorActive(int id, bool active)
{
    if (id < 0 || id >= m_monitors.size()) return false;
    
    UINT32 pathCount = 0;
    UINT32 modeCount = 0;
    if (GetDisplayConfigBufferSizes(QDC_ALL_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS) {
        return false;
    }

    QVector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
    QVector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

    if (QueryDisplayConfig(QDC_ALL_PATHS, &pathCount, paths.data(), &modeCount, modes.data(), NULL) != ERROR_SUCCESS) {
        return false;
    }

    // Match the monitor precisely using adapterId and targetId
    LUID targetAdapterId = m_monitors[id].adapterId;
    UINT32 targetId = m_monitors[id].targetId;
    
    bool found = false;
    for (UINT32 i = 0; i < pathCount; ++i) {
        if (paths[i].targetInfo.adapterId.LowPart == targetAdapterId.LowPart &&
            paths[i].targetInfo.adapterId.HighPart == targetAdapterId.HighPart &&
            paths[i].targetInfo.id == targetId) {
            
            if (active) {
                paths[i].flags |= DISPLAYCONFIG_PATH_ACTIVE;
            } else {
                paths[i].flags &= ~DISPLAYCONFIG_PATH_ACTIVE;
            }
            found = true;
            break;
        }
    }

    if (!found) return false;

    LONG result = SetDisplayConfig(pathCount, paths.data(), modeCount, modes.data(), 
                                  SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG | SDC_ALLOW_CHANGES);
    
    if (result == ERROR_SUCCESS) {
        refresh();
        return true;
    }
    return false;
}
