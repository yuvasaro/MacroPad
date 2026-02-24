#include "keystrokerecorder.h"
#include <iostream>

std::vector<KeyCode> KeystrokeRecorder::recordedKeys;
bool                 KeystrokeRecorder::isRecording = false;

bool  KeystrokeRecorder::IsRecording() { return isRecording; }
void  KeystrokeRecorder::Clear()       { recordedKeys.clear(); }

// ─── Mac ──────────────────────────────────────────────────────────────────────
#ifdef __APPLE__
#include <CoreGraphics/CGEventTypes.h>
#include <CoreGraphics/CGRemoteOperation.h>

CFMachPortRef      KeystrokeRecorder::eventTap      = nullptr;
CFRunLoopSourceRef KeystrokeRecorder::runLoopSource  = nullptr;

CGEventRef KeystrokeRecorder::EventCallback(CGEventTapProxy proxy, CGEventType type,
                                            CGEventRef event, void* refcon) {
    if (!isRecording) return event;
    if (type == kCGEventKeyDown || type == kCGEventFlagsChanged) {
        CGKeyCode keycode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
        recordedKeys.push_back(keycode);
        std::cout << "Recorded keycode: " << keycode << std::endl;
    }
    return event;
}

bool KeystrokeRecorder::StartRecording() {
    if (isRecording) return true;
    if (!AXIsProcessTrusted())
        std::cerr << "Not trusted for Accessibility. Grant permission to the app that launches this binary.\n";

    recordedKeys.clear();
    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventFlagsChanged);
    eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap,
                                kCGEventTapOptionDefault, eventMask, EventCallback, nullptr);
    if (!eventTap) {
        std::cerr << "Failed to create event tap. Check Accessibility permissions." << std::endl;
        return false;
    }
    runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);
    isRecording = true;
    std::cout << "Recording started..." << std::endl;
    return true;
}

std::vector<KeyCode> KeystrokeRecorder::StopRecording() {
    if (!isRecording) return recordedKeys;
    CGEventTapEnable(eventTap, false);
    if (runLoopSource) {
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
        CFRelease(runLoopSource); runLoopSource = nullptr;
    }
    if (eventTap) { CFRelease(eventTap); eventTap = nullptr; }
    isRecording = false;
    std::cout << "Recording stopped. Captured " << recordedKeys.size() << " keys." << std::endl;
    return recordedKeys;
}
#endif

// ─── Windows ──────────────────────────────────────────────────────────────────
#ifdef _WIN32
HHOOK  KeystrokeRecorder::keyboardHook = nullptr;
DWORD  KeystrokeRecorder::hookThreadId = 0;
HANDLE KeystrokeRecorder::hookThread   = nullptr;

LRESULT CALLBACK KeystrokeRecorder::HookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && isRecording && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        UINT vkCode = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam)->vkCode;
        recordedKeys.push_back(vkCode);
        std::cout << "Recorded keycode: " << vkCode << std::endl;
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

DWORD WINAPI KeystrokeRecorder::HookThreadProc(LPVOID) {
    hookThreadId = GetCurrentThreadId();
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, GetModuleHandle(nullptr), 0);
    if (!keyboardHook) {
        std::cerr << "Failed to create event tap. Check Accessibility permissions." << std::endl;
        return 1;
    }
    std::cout << "Recording started..." << std::endl;
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) > 0) { TranslateMessage(&msg); DispatchMessage(&msg); }
    if (keyboardHook) { UnhookWindowsHookEx(keyboardHook); keyboardHook = nullptr; }
    return 0;
}

bool KeystrokeRecorder::StartRecording() {
    if (isRecording) return true;
    recordedKeys.clear();
    isRecording = true;
    hookThread = CreateThread(nullptr, 0, HookThreadProc, nullptr, 0, nullptr);
    if (!hookThread) {
        std::cerr << "Failed to create event tap. Check Accessibility permissions." << std::endl;
        isRecording = false; return false;
    }
    Sleep(50); // let the hook thread install before returning
    return true;
}

std::vector<KeyCode> KeystrokeRecorder::StopRecording() {
    if (!isRecording) return recordedKeys;
    isRecording = false;
    if (hookThreadId) { PostThreadMessage(hookThreadId, WM_QUIT, 0, 0); hookThreadId = 0; }
    if (hookThread)   { WaitForSingleObject(hookThread, 2000); CloseHandle(hookThread); hookThread = nullptr; }
    std::cout << "Recording stopped. Captured " << recordedKeys.size() << " keys." << std::endl;
    return recordedKeys;
}
#endif
