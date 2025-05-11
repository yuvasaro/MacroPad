#ifndef HOTKEYHANDLER_H
#define HOTKEYHANDLER_H

#include <QString>
#include "profile.h"
#include "apptracker.h"
#include <QQmlListProperty>

#ifdef _WIN32
#include <windows.h>
#include <unordered_map>
#include <functional>
#elif __APPLE__
#include <Carbon/Carbon.h>
#elif __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#endif

class HotkeyHandler : public QObject {
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<Profile> profiles READ getProfiles NOTIFY profilesChanged)
    Q_PROPERTY(Profile* profileManager READ getProfileManager WRITE setProfileManager NOTIFY profileManagerChanged)

public:
    explicit HotkeyHandler(QObject* parent = nullptr);
    ~HotkeyHandler();

    static void registerGlobalHotkey(Profile* profile, int keyNum, const QString& type, const QString& content);
    static Profile* profileManager;
    static Profile* currentProfile;
    void initializeProfiles();
    void switchCurrentProfile(const QString& appName);

    Q_INVOKABLE QQmlListProperty<Profile> getProfiles();
    static qsizetype profileCount(QQmlListProperty<Profile> *list);
    static Profile* profileAt(QQmlListProperty<Profile> *list, qsizetype index);

    Profile* getProfileManager() { return profileManager; };
    void setProfileManager(Profile* profile);
    static void executeHotkey(int hotKeyNum, Profile* profileInstance);

    static void volumeUp();
    static void volumeDown();
    static void mute();
    static void scrollUp();
    static void scrollDown();

signals:
    void profilesChanged();
    void profileManagerChanged();

private:
    QList<Profile*> profiles;
    static QMap<int, EventHotKeyRef> registeredHotkeys;

#ifdef _WIN32
    static LRESULT CALLBACK hotkeyCallback(int nCode, WPARAM wParam, LPARAM lParam);
    static HHOOK keyboardHook;
    static std::unordered_map<UINT, std::function<void()>> hotkeyActions;
    static std::wstring wpathExec;

#elif __APPLE__
    static OSStatus hotkeyCallback(EventHandlerCallRef nextHandler, EventRef event, void *userData);
#elif __linux__
    static void listenForHotkeys();
    static Display* display;
#endif
};

#endif // HOTKEYHANDLER_H
