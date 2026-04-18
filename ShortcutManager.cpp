#include "ShortcutManager.h"
#include <QApplication>
#include <QDebug>

ShortcutManager::ShortcutManager(QObject *parent) : QObject(parent)
{
    qApp->installNativeEventFilter(this);
}

ShortcutManager::~ShortcutManager()
{
    for (int id : m_shortcuts.keys()) {
        unregisterShortcut(id);
    }
}

bool ShortcutManager::registerShortcut(int id, UINT modifiers, UINT vk)
{
    if (RegisterHotKey(NULL, id, modifiers, vk)) {
        m_shortcuts[id] = id;
        return true;
    }
    return false;
}

void ShortcutManager::unregisterShortcut(int id)
{
    if (m_shortcuts.contains(id)) {
        UnregisterHotKey(NULL, id);
        m_shortcuts.remove(id);
    }
}

bool ShortcutManager::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
        MSG *msg = static_cast<MSG *>(message);
        if (msg->message == WM_HOTKEY) {
            emit shortcutActivated(static_cast<int>(msg->wParam));
            return true;
        }
    }
    return false;
}
