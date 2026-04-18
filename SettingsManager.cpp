#include "SettingsManager.h"
#include <QCoreApplication>
#include <QDir>

#ifdef Q_OS_WIN
#include <QSettings>
#define REG_RUN "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#endif

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent), m_settings("QtScreenSwitcher", "QtScreenSwitcher")
{
}

void SettingsManager::setAutoStart(bool enable)
{
#ifdef Q_OS_WIN
    QSettings settings(REG_RUN, QSettings::NativeFormat);
    if (enable) {
        QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        settings.setValue("QtScreenSwitcher", "\"" + appPath + "\"");
    } else {
        settings.remove("QtScreenSwitcher");
    }
#endif
    m_settings.setValue("autoStart", enable);
}

bool SettingsManager::isAutoStart() const
{
    return m_settings.value("autoStart", false).toBool();
}

void SettingsManager::setTheme(const QString& theme)
{
    m_settings.setValue("theme", theme);
}

QString SettingsManager::theme() const
{
    return m_settings.value("theme", "light").toString();
}
