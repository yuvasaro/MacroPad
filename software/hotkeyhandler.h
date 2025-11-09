#ifndef HOTKEYHANDLER_H
#define HOTKEYHANDLER_H

#include <QString>
#include "profile.h"
#include "knobhandler.h"
#include <QQmlListProperty>
#include "serialhandler.h"

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

    static Profile* profileManager; // the profile that is selected from the dropdown in the UI
    static Profile* currentProfile; // the profile that matches the name of the application the user is on

    void initializeProfiles();
    void switchCurrentProfile(const QString& appName);

    Profile* getProfileManager() { return profileManager; };
    void setProfileManager(Profile* profile);
    void setSerialHandler(SerialHandler *s) { serialHandler = s; }
\
    Q_INVOKABLE QQmlListProperty<Profile> getProfiles();
    static qsizetype profileCount(QQmlListProperty<Profile> *list);
    static Profile* profileAt(QQmlListProperty<Profile> *list, qsizetype index);

    static void executeHotkey(int hotKeyNum, Profile* profileInstance);
    static void executeEncoder(int hotKeyNum, Profile* profileInstance, int id);


    static void registerGlobalHotkey(Profile* profile, int keyNum, const QString& type, const QString& content);

#ifdef __APPLE__
    static void pressAndReleaseKeys(const QStringList& keys);
#endif

signals:
    void profilesChanged();
    void profileManagerChanged();

private:
    QList<Profile*> profiles;
    SerialHandler *serialHandler {nullptr};

#ifdef _WIN32
    static LRESULT CALLBACK hotkeyCallback(int nCode, WPARAM wParam, LPARAM lParam);
    static HHOOK keyboardHook;
    static std::unordered_map<UINT, std::function<void()>> hotkeyActions;
    static std::wstring wpathExec;
    static bool findAndSwitchToWindow(const QString& exeName);
#elif __APPLE__
    static QMap<int, EventHotKeyRef> registeredHotkeys;
    static OSStatus hotkeyCallback(EventHandlerCallRef nextHandler, EventRef event, void *userData);
#elif __linux__
    static void listenForHotkeys();
    static Display* display;
#endif
};

#endif // HOTKEYHANDLER_H
