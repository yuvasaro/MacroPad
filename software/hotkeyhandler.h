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

#ifdef _WIN32
    static LRESULT CALLBACK hotkeyCallback(int nCode, WPARAM wParam, LPARAM lParam);
    static HHOOK keyboardHook;
    static std::unordered_map<UINT, std::function<void()>> hotkeyActions;
#elif __APPLE__
    static OSStatus hotkeyCallback(EventHandlerCallRef nextHandler, EventRef event, void *userData);
#elif __linux__
    static void listenForHotkeys();
    static Display* display;
#endif
};

#endif // HOTKEYHANDLER_H
