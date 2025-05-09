#ifndef HOTKEYHANDLER_H
#define HOTKEYHANDLER_H

#include <QString>
#include "profile.h"

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

class HotkeyHandler {
public:
    static void registerGlobalHotkey(Profile* profile, int keyNum, const QString& type, const QString& content);
    static Profile* profileManager;
    static void executeHotkey(int hotKeyNum, Profile* profileInstance);

    //volume controls
    static void volumeUp();
    static void volumeDown();
    static void mute();

    //scrolling controls
    static void scrollUp();
    static void scrollDown();
    static void autoScrollToggle();


    // Brightness controls
    static void brightnessUp();
    static void brightnessDown();
    static void brightnessToggle();

    // App‐switcher controls
    static void activateAppSwitcher();
    static void switchAppRight();
    static void switchAppLeft();

    //Zoom In and Out controls
    static void zoomIn();
    static void zoomOut();
    static void zoomReset();

    //browser tab switching controls
    static void nextTab();
    static void previousTab();


private:
    // Low‐level key injection
    static void sendSingleKey(WORD key);
    static void sendKeyCombo(WORD modifier, WORD key);

    // Tracks whether Task View is open
    static bool appSwitcherActive;


#ifdef _WIN32
    static LRESULT CALLBACK hotkeyCallback(int nCode, WPARAM wParam, LPARAM lParam);
    static HHOOK keyboardHook;
    static std::unordered_map<UINT, std::function<void()>> hotkeyActions;
    static std::wstring wpathExec;

#elif __APPLE__
    static QMap<int, EventHotKeyRef> registeredHotkeys;
    static OSStatus hotkeyCallback(EventHandlerCallRef nextHandler, EventRef event, void *userData);
#elif __linux__
    static void listenForHotkeys();
    static Display* display;
#endif
};

#endif // HOTKEYHANDLER_H
