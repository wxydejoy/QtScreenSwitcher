#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>

class SettingsManager : public QObject
{
    Q_OBJECT
public:
    explicit SettingsManager(QObject *parent = nullptr);

    void setAutoStart(bool enable);
    bool isAutoStart() const;

    void setTheme(const QString& theme);
    QString theme() const;

private:
    QSettings m_settings;
};

#endif // SETTINGSMANAGER_H
