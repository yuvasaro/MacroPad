#pragma once
#include <string>
#include <vector>

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
using KeyCode = CGKeyCode;
#endif

#ifdef _WIN32
#include <windows.h>
using KeyCode = UINT;
#endif

class KeystrokeRecorder {
public:
    static bool StartRecording();
    static std::vector<KeyCode> StopRecording();
    static bool IsRecording();
    static void Clear();
    static std::string ToString(const std::vector<KeyCode>& keys);

private:
    static std::vector<KeyCode> recordedKeys;
    static bool isRecording;

#ifdef __APPLE__
    static CGEventRef EventCallback(CGEventTapProxy, CGEventType, CGEventRef, void*);
    static CFMachPortRef eventTap;
    static CFRunLoopSourceRef runLoopSource;
#endif

#ifdef _WIN32
    static HHOOK keyboardHook;
    static DWORD hookThreadId;
    static HANDLE hookThread;
    static LRESULT CALLBACK HookCallback(int, WPARAM, LPARAM);
    static DWORD WINAPI HookThreadProc(LPVOID);
#endif
};
