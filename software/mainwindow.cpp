#include "mainwindow.h"
#include "QApplication"
#include "QIcon"
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QDebug>
#include "shellapi.h"
#include "serialhandler.h"

#ifdef _WIN32
#include <Windows.h>
#include <thread>
HHOOK MainWindow::keyboardHook = nullptr;
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    trayIcon(new QSystemTrayIcon(this)),
    trayMenu(new QMenu(this)),
    m_serialHandler(new SerialHandler(this))
{
    registerGlobalHotkey();
    createTrayIcon();
    connect(m_serialHandler, &SerialHandler::buttonPressed,
            this, &MainWindow::handleButtonPress);

    setWindowTitle("Configuration Software");
}

MainWindow::~MainWindow() {}

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

std::string path = "C:\\Users\\aarav\\AppData\\Local\\Programs\\Notion\\Notion.exe";
std::wstring wpath(path.begin(), path.end());  // Convert std::string to std::wstring

// Helper function to simulate Alt+Space keystroke
void performAltSpace() {
    qDebug() << "performAltSpace called";
    INPUT inputs[4] = {};

    // Press ALT key
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_MENU; // Alt key

    // Press SPACE key
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_SPACE; // Space key

    // Release SPACE key
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = VK_SPACE;
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    // Release ALT key
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_MENU;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(4, inputs, sizeof(INPUT));
}

LRESULT CALLBACK MainWindow::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        if (wParam == WM_KEYDOWN && kbdStruct->vkCode == VK_F5) {
            qDebug() << "F5 detected in global hook";
            // F5 pressed: simulate Alt+Space in a separate thread
            std::thread([] {
                performAltSpace();
            }).detach();
        }
        if (wParam == WM_KEYDOWN && kbdStruct->vkCode == VK_F6) {
            qDebug() << "F6 detected in global hook";
            // F6 pressed: execute existing functionality (launch app)
            std::thread([] {
                ShellExecuteW(NULL, L"open", wpath.c_str(), NULL, NULL, SW_SHOWNORMAL);
            }).detach();
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void MainWindow::registerGlobalHotkey() {
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    if (!keyboardHook) {
        qDebug() << "Failed to set global hotkey hook:" << GetLastError();
    } else {
        qDebug() << "Global hotkey hook registered successfully.";
    }
}

void MainWindow::handleButtonPress(int button) {
    qDebug() << "handleButtonPress called with button:" << button;
    if (button == 1) {
        // Arduino button 1 pressed: simulate Alt+Space in a separate thread
        std::thread([] {
            performAltSpace();
        }).detach();
    }
}

#endif

// ===== MACOS IMPLEMENTATION =====
#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#include <ApplicationServices/ApplicationServices.h>
#include <iostream>

OSStatus MainWindow::hotkeyCallback(EventHandlerCallRef nextHandler, EventRef event, void *userData) {
    std::cout << "Tilde (~) key pressed! Triggering Cmd+Space..." << std::endl;
    system("osascript -e 'tell application \"System Events\" to key code 49 using command down'");
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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XTest.h>

void MainWindow::listenForHotkeys() {
    XEvent event;
    while (true) {
        XNextEvent(display, &event);
        if (event.type == KeyPress) {
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
