#include "mainwindow.h"
#include "QApplication"
#include "QIcon"
#include <QAction>
#include <QMenu>
#include "string"

#include "shellapi.h"
#include <QMainWindow>
#include <windows.h>
#include <functional>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include "shellapi.h"
HHOOK MainWindow::keyboardHook = nullptr;
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), trayIcon(new QSystemTrayIcon(this)), trayMenu(new QMenu(this)) {

    registerGlobalHotkey();  // This will set the keyboard hook properly
    createTrayIcon();

    setWindowTitle("Configuration Software");
}

MainWindow::~MainWindow() {
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = nullptr;
    }
}

void MainWindow::createTrayIcon() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::warning(this, "Warning", "System tray is not available!");
        return;
    }

    // Set a valid icon (adjust path accordingly)
    trayIcon->setIcon(QIcon(":/icons/app_icon.png"));
    trayIcon->setToolTip("Configuration Software Running");

    // Create tray menu actions
    QAction *restoreAction = new QAction("Show Window", this);
    QAction *exitAction = new QAction("Exit", this);

    connect(restoreAction, &QAction::triggered, this, &MainWindow::showWindow);
    connect(exitAction, &QAction::triggered, this, &MainWindow::exitApplication);

    // Add actions to menu
    trayMenu->addAction(restoreAction);
    trayMenu->addAction(exitAction);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
}


void MainWindow::closeEvent(QCloseEvent *event) {
    if (trayIcon->isVisible()) {
        hide();  // Hide the window
        event->ignore();  // Ignore the close event
    }
}

void MainWindow::showWindow() {
    showNormal();  // Restore window
    activateWindow();
}

void MainWindow::exitApplication() {
    trayIcon->hide();  // Hide tray icon before quitting
    QApplication::quit();
}
// ===== WINDOWS IMPLEMENTATION =====
#ifdef _WIN32
#include <thread>

unsigned long int customVkCode = VK_F6; // Default hotkey (changeable)
std::vector<INPUT> inputs;


//std::string path = "C:\\Users\\aarav\\OneDrive\\Desktop\\Arduino IDE.lnk";
std::string path = "notepad";

std::wstring wpath(path.begin(), path.end());  // Convert std::string to std::wstring


// LRESULT CALLBACK MainWindow::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
//     if (nCode == HC_ACTION) {
//         KBDLLHOOKSTRUCT *kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
//         if (wParam == WM_KEYDOWN && kbdStruct->vkCode == VK_F5) {
//             // Run the notification in a separate thread to prevent blocking
//             std::thread([] {
//                 // Simulate Alt + Space
//                 INPUT inputs[4] = {};

//                 // Press ALT
//                 inputs[0].type = INPUT_KEYBOARD;
//                 inputs[0].ki.wVk = VK_MENU;

//                 // Press SPACE
//                 inputs[1].type = INPUT_KEYBOARD;
//                 inputs[1].ki.wVk = VK_SPACE;

//                 // Release SPACE
//                 inputs[2].type = INPUT_KEYBOARD;
//                 inputs[2].ki.wVk = VK_SPACE;
//                 inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

//                 // Release ALT
//                 inputs[3].type = INPUT_KEYBOARD;
//                 inputs[3].ki.wVk = VK_MENU;
//                 inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

//                 SendInput(4, inputs, sizeof(INPUT));
//             }).detach();
//         }
//         if (wParam == WM_KEYDOWN && kbdStruct->vkCode == VK_F6) {  // Trigger hotkey
//             std::thread([] {
//                 // Use ShellExecuteW to open the application instantly
//                 ShellExecuteW(NULL, L"open", wpath.c_str(), NULL, NULL, SW_SHOWNORMAL);
//             }).detach();
//         }
//     }
//     return CallNextHookEx(NULL, nCode, wParam, lParam);
// }

// Register a hotkey and associate it with an action
void MainWindow::RegisterHotkey(UINT vkCode, std::function<void()> action) {
    hotkeyActions[vkCode] = action;
}

// Simulates keypress sequence (e.g., Alt + Space)
void MainWindow::doTasks(std::vector<INPUT>& inputs) {
    inputs.resize(4);

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_MENU;  // Press ALT

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_SPACE;  // Press SPACE

    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = VK_SPACE;
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;  // Release SPACE

    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_MENU;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;  // Release ALT
}

// Hook to detect global hotkeys
LRESULT CALLBACK MainWindow::KeyCustomization(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN) {
            auto it = hotkeyActions.find(kbdStruct->vkCode);
            if (it != hotkeyActions.end()) {
                std::thread(it->second).detach();  // Run action in separate thread
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Registers the global keyboard hook
void MainWindow::registerGlobalHotkey() {
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyCustomization, GetModuleHandle(NULL), 0);
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
