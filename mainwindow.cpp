#include "mainwindow.h"

#ifdef _WIN32
HHOOK MainWindow::keyboardHook = nullptr;
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    registerGlobalHotkey();
}

MainWindow::~MainWindow() {}

// ===== WINDOWS IMPLEMENTATION =====
#ifdef _WIN32
#include <thread>
#include <chrono>

LRESULT CALLBACK MainWindow::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN && kbdStruct->vkCode == VK_F5) {
            // Run the notification in a separate thread to prevent blocking
            std::thread([] {
                MessageBox(NULL, L"Hotkey F5 Pressed!", L"Notification", MB_OK | MB_SYSTEMMODAL);
            }).detach();
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void MainWindow::registerGlobalHotkey() {
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
}
#endif

// ===== MACOS IMPLEMENTATION =====
#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#include <ApplicationServices/ApplicationServices.h>
#include <iostream>

OSStatus MainWindow::hotkeyCallback(EventHandlerCallRef nextHandler, EventRef event, void *userData) {
    std::cout << "Tilde (~) key pressed! Triggering Cmd+Space..." << std::endl;

    // Simulate Cmd+Space keystroke
    // CGEventRef cmdDown = CGEventCreateKeyboardEvent(NULL, kVK_Command, true);
    // CGEventRef spaceDown = CGEventCreateKeyboardEvent(NULL, kVK_Space, true);
    // CGEventRef spaceUp = CGEventCreateKeyboardEvent(NULL, kVK_Space, false);
    // CGEventRef cmdUp = CGEventCreateKeyboardEvent(NULL, kVK_Command, false);

    // CGEventPost(kCGHIDEventTap, cmdDown);
    // CGEventPost(kCGHIDEventTap, spaceDown);
    // CGEventPost(kCGHIDEventTap, spaceUp);
    // CGEventPost(kCGHIDEventTap, cmdUp);

    // CFRelease(cmdDown);
    // CFRelease(spaceDown);
    // CFRelease(spaceUp);
    // CFRelease(cmdUp);

    system("osascript -e 'tell application \"System Events\" to key code 49 using command down'"); // 49 = Space key

    return noErr;
}

void MainWindow::registerGlobalHotkey() {
    std::cout << "Registering tilde (~) as a global hotkey..." << std::endl;

    EventHotKeyRef hotKeyRef;
    EventHotKeyID hotKeyID;
    EventTypeSpec eventType;
    eventType.eventClass = kEventClassKeyboard;
    eventType.eventKind = kEventHotKeyPressed;

    hotKeyID.signature = 'htk1';
    hotKeyID.id = 1;

    // Register `~` as the hotkey
    OSStatus status = RegisterEventHotKey(kVK_ANSI_Grave, 0, hotKeyID, GetApplicationEventTarget(), 0, &hotKeyRef);
    if (status != noErr) {
        std::cerr << "Failed to register hotkey. Error code: " << status << std::endl;
    } else {
        std::cout << "Hotkey registered successfully! Press ~ to trigger Cmd+Space." << std::endl;
    }

    InstallApplicationEventHandler(&hotkeyCallback, 1, &eventType, nullptr, nullptr);
}
#endif


// ===== LINUX IMPLEMENTATION =====
#ifdef __linux__
#include <cstdlib>

void MainWindow::listenForHotkeys() {
    XEvent event;
    while (true) {
        XNextEvent(display, &event);
        if (event.type == KeyPress) {
            // Use 'notify-send' to show a notification without stealing focus
            system("notify-send 'MacroPad' 'Hotkey F5 Pressed!'");
        }
    }
}

void MainWindow::registerGlobalHotkey() {
    display = XOpenDisplay(NULL);
    if (!display) return;

    Window root = DefaultRootWindow(display);
    KeyCode keycode = XKeysymToKeycode(display, XK_F5);

    XGrabKey(display, keycode, AnyModifier, root, True, GrabModeAsync, GrabModeAsync);
    listenForHotkeys();
}
#endif
