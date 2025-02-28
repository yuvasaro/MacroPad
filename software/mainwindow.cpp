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
    connect(m_serialHandler, &SerialHandler::dataReceived,
            this, &MainWindow::onDataReceived);

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

std::string pathNotion = "C:\\Users\\aarav\\AppData\\Local\\Programs\\Notion\\Notion.exe";
std::wstring wpathNotion(pathNotion.begin(), pathNotion.end());  // Convert std::string to std::wstring

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

// Simulates Ctrl+Shift+` (backtick on a US keyboard)
static void performCtrlShiftBacktick()
{
    qDebug() << "performCtrlShiftBacktick called";

    // VK_OEM_3 is the virtual-key code for the '`' key on US keyboards
    // (the same key that produces '~' when shifted)
    const SHORT KEY_BACKTICK = VK_OEM_3;

    INPUT inputs[6] = {};

    // Press CTRL
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;

    // Press SHIFT
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_SHIFT;

    // Press ` (backtick)
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = KEY_BACKTICK;

    // Release ` (backtick)
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = KEY_BACKTICK;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    // Release SHIFT
    inputs[4].type = INPUT_KEYBOARD;
    inputs[4].ki.wVk = VK_SHIFT;
    inputs[4].ki.dwFlags = KEYEVENTF_KEYUP;

    // Release CTRL
    inputs[5].type = INPUT_KEYBOARD;
    inputs[5].ki.wVk = VK_CONTROL;
    inputs[5].ki.dwFlags = KEYEVENTF_KEYUP;

    // Send the sequence
    SendInput(6, inputs, sizeof(INPUT));
}

// Simulates Ctrl+Shift+D
static void performCtrlShiftD()
{
    qDebug() << "performCtrlShiftD called";

    INPUT inputs[6] = {};

    // Press CTRL
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;

    // Press SHIFT
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_SHIFT;

    // Press 'D'
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'D';

    // Release 'D'
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = 'D';
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    // Release SHIFT
    inputs[4].type = INPUT_KEYBOARD;
    inputs[4].ki.wVk = VK_SHIFT;
    inputs[4].ki.dwFlags = KEYEVENTF_KEYUP;

    // Release CTRL
    inputs[5].type = INPUT_KEYBOARD;
    inputs[5].ki.wVk = VK_CONTROL;
    inputs[5].ki.dwFlags = KEYEVENTF_KEYUP;

    // Send the sequence
    SendInput(6, inputs, sizeof(INPUT));
}

// Simulates pressing and releasing the "Next Track" multimedia key.
static void skipTrack()
{
    qDebug() << "skipTrack called";
    INPUT inputs[2] = {};

    // Press the VK_MEDIA_NEXT_TRACK key
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_MEDIA_NEXT_TRACK;

    // Release the VK_MEDIA_NEXT_TRACK key
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_MEDIA_NEXT_TRACK;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    // Send the key events
    SendInput(2, inputs, sizeof(INPUT));
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
                //ShellExecuteW(NULL, L"open", wpath.c_str(), NULL, NULL, SW_SHOWNORMAL);
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

// void launchProgram(std::wstring wpath)
// {
//     //std::wstring wpath(path.begin(), path.end());  // Convert std::string to std::wstring
//     // Launch an app
//     std::thread([] {
//         extern std::wstring wpath; // from your code
//         ShellExecuteW(NULL, L"open", wpath.c_str(), NULL, NULL, SW_SHOWNORMAL);
//     }).detach();
// }

std::string pathVSCode = "C:\\Users\\aarav\\AppData\\Local\\Programs\\Microsoft VS Code\\Code.exe";
std::wstring wpathVSCode(pathVSCode.begin(), pathVSCode.end());  // Convert std::string to std::wstring

std::string pathArduino = "C:\\Users\\aarav\\AppData\\Local\\Programs\\Arduino IDE\\Arduino IDE.exe";
std::wstring wpathArduino(pathArduino.begin(), pathArduino.end());  // Convert std::string to std::wstring

std::string pathGitHub = "C:\\Users\\aarav\\AppData\\Local\\GitHubDesktop\\GitHubDesktop.exe";
std::wstring wpathGitHub(pathGitHub.begin(), pathGitHub.end());  // Convert std::string to std::wstring

// --------------------------
//   New Slot Implementation
// --------------------------
int profile =1;
void MainWindow::onDataReceived(int number)
{
    qDebug() << "onDataReceived: " << number;
    if(number>10)
    {
        profile = number-10;
        qDebug() << "Profile switched to " << profile;
    }

    switch(profile)
    {
    case 1:
        qDebug() << "Profile : " << profile;

        if (number == 1) {
            // launch VSCode
            std::thread([] {
                extern std::wstring wpathVSCode; // from your code
                ShellExecuteW(NULL, L"open", wpathVSCode.c_str(), NULL, NULL, SW_SHOWNORMAL);
            }).detach();
            qDebug()<< "VSCode launched";
        }
        else if (number == 2) {
            //launch Arduino
            std::thread([] {
                extern std::wstring wpathArduino; // from your code
                ShellExecuteW(NULL, L"open", wpathArduino.c_str(), NULL, NULL, SW_SHOWNORMAL);
            }).detach();
            qDebug()<< "Arduino launched";
        }
        // Add more conditions as needed
        else if (number == 3) {
            //should launch Spotify

            // Simulate Alt+Space in a separate thread
            std::thread([] {
                performAltSpace();
            }).detach();
            qDebug()<< "Spotify launched";
        }
        else if (number == 4) {
            //launchProgram(wpathNotion);
            std::thread([] {
                extern std::wstring wpathNotion; // from your code
                ShellExecuteW(NULL, L"open", wpathNotion.c_str(), NULL, NULL, SW_SHOWNORMAL);
            }).detach();
            qDebug()<< "Notion launched";
        }
        // Add more conditions as needed
        break;
    case 2: // VSCode profile
        qDebug() << "Profile : " << profile;

        if (number == 1) {
            // open Terminal
            std::thread([] {
                performCtrlShiftBacktick();
            }).detach();
            qDebug()<< "Terminal opened";
        }
        else if (number == 2) {
            //run and debug
            std::thread([] {
                performCtrlShiftD();
            }).detach();
            qDebug()<< "run and debug launched";
        }
        // Add more conditions as needed
        else if (number == 3) {
            //launch GitHub
            std::thread([] {
                extern std::wstring wpathGitHub; // from your code
                ShellExecuteW(NULL, L"open", wpathGitHub.c_str(), NULL, NULL, SW_SHOWNORMAL);
            }).detach();

            qDebug()<< "GitHub launched";
        }
        else if (number == 4) {
            //skip Track
            std::thread([] {
                skipTrack();
            }).detach();
            qDebug()<< "Track skipped";
        }
        // Add more conditions as needed
        break;
    default:
        break;
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
