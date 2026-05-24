#include "keystrokerecorder.h"
#include <algorithm>
#include <iostream>
#include <unordered_map>

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

    if (type == kCGEventTapDisabledByTimeout || type == kCGEventTapDisabledByUserInput) {
        CGEventTapEnable(eventTap, true);
        return nullptr;
    }

    auto appendKeyIfMissing = [](CGKeyCode recordedKey) {
        if (std::find(recordedKeys.begin(), recordedKeys.end(), recordedKey) == recordedKeys.end()) {
            recordedKeys.push_back(recordedKey);
        }
    };

    if (type == kCGEventKeyDown || type == kCGEventFlagsChanged) {
        CGKeyCode keycode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
        appendKeyIfMissing(keycode);

        std::cout << "Recorded keys down:";
        for (CGKeyCode recordedKey : recordedKeys) {
            std::cout << " " << recordedKey;
        }
        std::cout << std::endl;
    }

    if (type == kCGEventKeyDown || type == kCGEventKeyUp || type == kCGEventFlagsChanged) {
        return nullptr;
    }
    return event;
}

bool KeystrokeRecorder::StartRecording() {
    if (isRecording) return true;
    if (!AXIsProcessTrusted())
        std::cerr << "Not trusted for Accessibility. Grant permission to the app that launches this binary.\n";

    if (!CGPreflightListenEventAccess()) {
        std::cerr << "Input Monitoring permission is required to capture and block system shortcuts while recording.\n";
        CGRequestListenEventAccess();
        return false;
    }

    recordedKeys.clear();

    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown)
                           | CGEventMaskBit(kCGEventKeyUp)
                           | CGEventMaskBit(kCGEventFlagsChanged);
    eventTap = CGEventTapCreate(
        kCGHIDEventTap,
        kCGHeadInsertEventTap,
        kCGEventTapOptionDefault,
        eventMask,
        EventCallback,
        nullptr
        );

    if (!eventTap) {
        std::cerr << "Failed to create event tap. Check Accessibility and Input Monitoring permissions." << std::endl;
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

std::string KeystrokeRecorder::ToString(const std::vector<KeyCode>& keys) {
    static const std::unordered_map<CGKeyCode, std::string> keyMap = {
                                                                      // Modifiers
                                                                      {kVK_Command,      "cmd"},
                                                                      {kVK_Shift,        "shift"},   {kVK_RightShift,   "shift"},
                                                                      {kVK_Control,      "ctrl"},    {kVK_RightControl, "ctrl"},
                                                                      {kVK_Option,       "option"},  {kVK_RightOption,  "option"},
                                                                      {kVK_Function,     "fn"},
                                                                      // Special
                                                                      {kVK_Space,        "space"},
                                                                      {kVK_Tab,          "tab"},
                                                                      {kVK_Return,       "return"},
                                                                      {kVK_Delete,       "delete"},
                                                                      {kVK_ForwardDelete,"forwarddelete"},
                                                                      {kVK_Escape,       "esc"},
                                                                      {kVK_LeftArrow,    "left"},    {kVK_RightArrow,   "right"},
                                                                      {kVK_UpArrow,      "up"},      {kVK_DownArrow,    "down"},
                                                                      {kVK_Home,         "home"},    {kVK_End,          "end"},
                                                                      {kVK_PageUp,       "pageup"},  {kVK_PageDown,     "pagedown"},
                                                                      // Function keys
                                                                      {kVK_F1,"f1"},{kVK_F2,"f2"},{kVK_F3,"f3"},{kVK_F4,"f4"},
                                                                      {kVK_F5,"f5"},{kVK_F6,"f6"},{kVK_F7,"f7"},{kVK_F8,"f8"},
                                                                      {kVK_F9,"f9"},{kVK_F10,"f10"},{kVK_F11,"f11"},{kVK_F12,"f12"},
                                                                      // Letters
                                                                      {kVK_ANSI_A,"a"},{kVK_ANSI_B,"b"},{kVK_ANSI_C,"c"},{kVK_ANSI_D,"d"},
                                                                      {kVK_ANSI_E,"e"},{kVK_ANSI_F,"f"},{kVK_ANSI_G,"g"},{kVK_ANSI_H,"h"},
                                                                      {kVK_ANSI_I,"i"},{kVK_ANSI_J,"j"},{kVK_ANSI_K,"k"},{kVK_ANSI_L,"l"},
                                                                      {kVK_ANSI_M,"m"},{kVK_ANSI_N,"n"},{kVK_ANSI_O,"o"},{kVK_ANSI_P,"p"},
                                                                      {kVK_ANSI_Q,"q"},{kVK_ANSI_R,"r"},{kVK_ANSI_S,"s"},{kVK_ANSI_T,"t"},
                                                                      {kVK_ANSI_U,"u"},{kVK_ANSI_V,"v"},{kVK_ANSI_W,"w"},{kVK_ANSI_X,"x"},
                                                                      {kVK_ANSI_Y,"y"},{kVK_ANSI_Z,"z"},
                                                                      // Numbers
                                                                      {kVK_ANSI_0,"0"},{kVK_ANSI_1,"1"},{kVK_ANSI_2,"2"},{kVK_ANSI_3,"3"},
                                                                      {kVK_ANSI_4,"4"},{kVK_ANSI_5,"5"},{kVK_ANSI_6,"6"},{kVK_ANSI_7,"7"},
                                                                      {kVK_ANSI_8,"8"},{kVK_ANSI_9,"9"},
                                                                      };

    // Modifiers sort before regular keys
    auto order = [](CGKeyCode k) -> int {
        switch(k) {
        case kVK_Command:                      return 0;
        case kVK_Control: case kVK_RightControl: return 1;
        case kVK_Option:  case kVK_RightOption:  return 2;
        case kVK_Shift:   case kVK_RightShift:   return 3;
        default:                               return 4;
        }
    };

    std::vector<KeyCode> sorted = keys;
    std::stable_sort(sorted.begin(), sorted.end(),
                     [&](KeyCode a, KeyCode b){ return order(a) < order(b); });

    std::string result;
    for (auto k : sorted) {
        auto it = keyMap.find(k);
        if (it == keyMap.end()) continue;
        if (!result.empty()) result += "+";
        result += it->second;
    }
    return result;
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

std::string KeystrokeRecorder::ToString(const std::vector<KeyCode>& keys) {
    static const std::unordered_map<UINT, std::string> keyMap = {
                                                                 // Modifiers
                                                                 {VK_LWIN,"Win"},    {VK_RWIN,"Win"},
                                                                 {VK_CONTROL,"Ctrl"},{VK_LCONTROL,"Ctrl"},{VK_RCONTROL,"Ctrl"},
                                                                 {VK_MENU,"Alt"},    {VK_LMENU,"Alt"},    {VK_RMENU,"Alt"},
                                                                 {VK_SHIFT,"Shift"}, {VK_LSHIFT,"Shift"}, {VK_RSHIFT,"Shift"},
                                                                 // Special
                                                                 {VK_SPACE,"Space"},   {VK_RETURN,"Enter"},  {VK_BACK,"Backspace"},
                                                                 {VK_TAB,"Tab"},       {VK_ESCAPE,"Esc"},    {VK_DELETE,"Delete"},
                                                                 {VK_CAPITAL,"Caps Lock"},
                                                                 {VK_LEFT,"Left"},     {VK_RIGHT,"Right"},
                                                                 {VK_UP,"Up"},         {VK_DOWN,"Down"},
                                                                 {VK_HOME,"Home"},     {VK_END,"End"},
                                                                 {VK_PRIOR,"PageUp"},  {VK_NEXT,"PageDown"},
                                                                 // Function keys
                                                                 {VK_F1,"F1"},{VK_F2,"F2"},{VK_F3,"F3"},{VK_F4,"F4"},
                                                                 {VK_F5,"F5"},{VK_F6,"F6"},{VK_F7,"F7"},{VK_F8,"F8"},
                                                                 {VK_F9,"F9"},{VK_F10,"F10"},{VK_F11,"F11"},{VK_F12,"F12"},
                                                                 };

    auto order = [](UINT vk) -> int {
        switch(vk) {
        case VK_LWIN: case VK_RWIN:                          return 0;
        case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL: return 1;
        case VK_MENU:    case VK_LMENU:    case VK_RMENU:    return 2;
        case VK_SHIFT:   case VK_LSHIFT:   case VK_RSHIFT:   return 3;
        default:                                              return 4;
        }
    };

    std::vector<KeyCode> sorted = keys;
    std::stable_sort(sorted.begin(), sorted.end(),
                     [&](KeyCode a, KeyCode b){ return order(a) < order(b); });

    std::string result;
    for (auto k : sorted) {
        std::string name;
        auto it = keyMap.find(k);
        if (it != keyMap.end()) {
            name = it->second;
        } else if (k >= 'A' && k <= 'Z') {
            name = std::string(1, (char)k);       // A–Z
        } else if (k >= '0' && k <= '9') {
            name = std::string(1, (char)k);       // 0–9
        } else {
            continue; // unknown key, skip
        }
        if (!result.empty()) result += "+";
        result += name;
    }
    return result;
}

#endif
