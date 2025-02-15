#include "mainwindow.h"
#include "QApplication"
#include "QIcon"
#include <QAction>
#include <QMenu>
#include "string"

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

//std::string path = "C:\\Users\\aarav\\OneDrive\\Desktop\\Arduino IDE.lnk";
std::string path = "Notepad";
std::wstring wpath(path.begin(), path.end());  // Convert std::string to std::wstring

//Registers a hotkey and associates it with an action (in this case, a lambda function that performs an action).
void MainWindow::RegisterHotkey(UINT vkCode, std::function<void()> action) {
    hotkeyActions[vkCode] = action;
}

/*Customized arbitrary set of keystrokes
This is an example: Simulates pressing the Alt + Space keys, shoudl open up the system menu in Windows.
*/
void MainWindow::simulateAltSpace() {
    std::vector<INPUT> inputs(4);

    // Press ALT
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_MENU;

    // Press Space
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_SPACE;

    // Release Space
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = VK_SPACE;
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    // Release ALT
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_MENU;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(inputs.size(), inputs.data(), sizeof(INPUT));
}


// KeyCustomization function: This callback function processes keyboard input for the global hotkeys
LRESULT CALLBACK MainWindow::KeyCustomization(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN) {
            auto it = hotkeyActions.find(kbdStruct->vkCode);
            if (it != hotkeyActions.end()) {
                std::thread(it->second).detach();  // Run the registered action in a separate thread
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


// registerGlobalHotkey function: This function registers the global hotkeys (F6, F7)
void MainWindow::registerGlobalHotkey() {

    // Register F6 to open Notepad (or any executable defined in 'path')
    RegisterHotkey(VK_F6, []() {
        ShellExecuteW(NULL, L"open", wpath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    });

    // Register F7 to simulate Ctrl+Alt
    RegisterHotkey(VK_F7, []() {
        std::thread([]() {
            // Simulate Control + Alt
            simulateAltSpace();
        }).detach();
    });

    // Set the keyboard hook to listen for key events globally (so the app runs)
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyCustomization, GetModuleHandle(NULL), 0);
}
#endif

// ===== MACOS IMPLEMENTATION =====
#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <QDebug>
#include <QProcess>
#include <QFileInfo>

static EventHotKeyRef hotKeyRef_Tilde;
static EventHotKeyRef hotKeyRef_Backslash;
static EventHotKeyID hotKeyID_Tilde;
static EventHotKeyID hotKeyID_Backslash;
static EventHandlerUPP eventHandlerUPP;

// Placeholder path to the executable
const QString EXECUTABLE_PATH = "/Users/yuvasaro/Developer/C/experiments/bits/swap/inplace_swap";  // Replace this!

OSStatus MainWindow::hotkeyCallback(EventHandlerCallRef nextHandler, EventRef event, void *userData) {
    EventHotKeyID hotKeyID;
    GetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, NULL, sizeof(hotKeyID), NULL, &hotKeyID);

    if (hotKeyID.id == 1) {
        qDebug() << "Tilde (~) key pressed! Opening Discord...";
        system("open -a 'Discord'");
    }
    else if (hotKeyID.id == 2) {
        qDebug() << "Backslash (\\) key pressed! Running executable at:" << EXECUTABLE_PATH;

        QProcess process;

        // Extract and set the working directory
        QFileInfo exeInfo(EXECUTABLE_PATH);
        QString workingDir = exeInfo.absolutePath();
        process.setWorkingDirectory(workingDir);

        // Print working directory for debugging
        qDebug() << "Setting working directory to:" << workingDir;

        // Launch the executable
        if (!process.startDetached(EXECUTABLE_PATH)) {
            qDebug() << "Failed to launch executable!";
        }
    }

    return noErr;
}

void MainWindow::registerGlobalHotkey() {
    qDebug() << "Registering Tilde (~) and Backslash (\\) as global hotkeys...";

    EventTypeSpec eventType;
    eventType.eventClass = kEventClassKeyboard;
    eventType.eventKind = kEventHotKeyPressed;

    hotKeyID_Tilde.signature = 'htk1';
    hotKeyID_Tilde.id = 1;
    hotKeyID_Backslash.signature = 'htk2';
    hotKeyID_Backslash.id = 2;

    // Create the event handler
    eventHandlerUPP = NewEventHandlerUPP(hotkeyCallback);
    InstallApplicationEventHandler(eventHandlerUPP, 1, &eventType, nullptr, nullptr);

    // Register "Tilde" key (kVK_ANSI_Grave corresponds to `~`)
    OSStatus status_Tilde = RegisterEventHotKey(kVK_ANSI_Grave, 0, hotKeyID_Tilde, GetApplicationEventTarget(), 0, &hotKeyRef_Tilde);

    // Register "Backslash" key (kVK_ANSI_Backslash corresponds to `\`)
    OSStatus status_Backslash = RegisterEventHotKey(kVK_ANSI_Backslash, 0, hotKeyID_Backslash, GetApplicationEventTarget(), 0, &hotKeyRef_Backslash);

    if (status_Tilde != noErr) {
        qDebug() << "Failed to register Tilde hotkey. Error code:" << status_Tilde;
    } else {
        qDebug() << "Tilde (~) hotkey registered successfully!";
    }

    if (status_Backslash != noErr) {
        qDebug() << "Failed to register Backslash hotkey. Error code:" << status_Backslash;
    } else {
        qDebug() << "Backslash (\\) hotkey registered successfully! Press \\ to run the executable.";
    }
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
