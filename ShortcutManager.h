#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QMap>
#include <windows.h>

class ShortcutManager : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit ShortcutManager(QObject *parent = nullptr);
    ~ShortcutManager();

    bool registerShortcut(int id, UINT modifiers, UINT vk);
    void unregisterShortcut(int id);

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

signals:
    void shortcutActivated(int id);

private:
    QMap<int, int> m_shortcuts;
};

#endif // SHORTCUTMANAGER_H
